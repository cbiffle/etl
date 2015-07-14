#ifndef _ETL_DATA_MAYBE_H_INCLUDED
#define _ETL_DATA_MAYBE_H_INCLUDED

/*
 * A Maybe<T> is a T-sized chunk of memory that may or may not contain useful
 * data.  It's intended as a replacement for passing around a (bool, address)
 * pair, just as RangePtr is intended as a replacement for (size, address)
 * pairs.  Like RangePtr, Maybe is designed to allow abstraction without
 * cost: it's designed to boil down to the code you would have written by
 * hand, with no penalty to code generation.
 *
 * For example, imagine a function that gets the latest altitude measurement,
 * if one is available.  Traditionally it might have the signature:
 *
 *   bool get_altitude(float *);
 *
 * This is both awkward and dangerous.  Awkward, because the altitude "out-
 * parameter" must be declared before the call and passed by address.
 * Dangerous, because code might mistakenly try to access the out-parameter's
 * value even if the function returns false.
 *
 * Maybe provides a safer way at approximately equal cost:
 *
 *   Maybe<float> get_altitude();
 *
 * Maybe is also designed to work with complex non-POD types.  An "empty" Maybe
 * (known as "nothing") can be passed around freely without ever invoking
 * T's constructors or assignment operators.  A "full" Maybe ("something")
 * has been constructed, and will destruct the T properly when the Maybe itself
 * is destroyed.  These guarantees are tedious to achieve with out-parameters.
 *
 * Maybe<T> has a special relationship with its sister type, Nothing.  Nothing
 * essentially acts like an empty Maybe<T> for any T, just as nullptr acts as
 * a null of any type.
 *
 *
 * Implementation Discussion
 * -------------------------
 *
 * This is a crapload of code for such a simple concept -- but then, perhaps
 * this is simply not a simple concept in C++.  Getting correct move semantics,
 * in-place construction, object lifecycle, etc. is nuanced and is simply not
 * possible to describe in languages like Haskell with simpler Maybe types.
 *
 * Maybe<T> is currently a hierarchy with three levels.
 *
 * The top is MaybeTag, a common base type that can be used to detect Maybe
 * types regardless of parameterization.
 *
 * Beneath this is *either* MaybeBaseNT or MaybeBaseT, depending on whether T
 * has a non-trivial or trivial destructor, respectively.  These customize the
 * available set of constructors, allowing Maybe<T> for T with trivial
 * destructor to function as a literal type.
 *
 * The final layer is Maybe<T> itself, which factors out common code from the
 * two base implementations.
 *
 * The overall factoring is inspired by Andrzej Krzemienski's work on
 * optional<T>, though the implementation differs significantly in terms of
 * what it permits for literal types and what it requires (and does not require)
 * to be constexpr for non-literal types.
 */

#include <new>
#include <type_traits>

#include "etl/assert.h"
#include "etl/attribute_macros.h"
#include "etl/implicit.h"
#include "etl/invoke.h"
#include "etl/type_traits.h"
#include "etl/utility.h"

namespace etl {
namespace data {

/*******************************************************************************
 * Prerequisites and forward declarations.
 */

/*
 * Nothing is the type of the polymorphic constant 'nothing', a stand-in for
 * an empty Maybe<T> for any T.
 */
constexpr struct Nothing {} nothing {};

/*
 * InPlace is the type of the tag 'in_place', which distinguishes forwarded
 * constructors from Maybe's own constructors.
 */
constexpr struct InPlace {} in_place {};

/*
 * MaybeTag is the root of the Maybe<T> hierarchy.
 */
struct MaybeTag {};

/*
 * IsMaybe<T> is a trait type that can recognize Maybes.
 */
template <typename T>
struct IsMaybe : public std::is_base_of<MaybeTag, Invoke<std::decay<T>>> {};

/*
 * Forward declaration of default check policy.
 */
struct LaxMaybeCheckPolicy;


/*******************************************************************************
 * MaybeBaseT and MaybeBaseNT.
 */

/*
 * Base implementation of Maybe<T> for T with non-trivial destructor.
 *
 * Contains:
 * - constexpr empty-constructors,
 * - *non-constexpr* full constructors,
 * - a non-trivial destructor.
 *
 * You may be wondering why we have constexpr constructors in a type with
 * a non-trivial destructor; it's so that static instances can be initialized
 * by image-copy instead of a constructor call.  Every little bit helps.
 */
template <typename T>
struct MaybeBaseNT : MaybeTag {
  union Storage {
    struct {} dummy;
    T object;

    constexpr Storage() : dummy{} {}

    template <typename ... Args>
    Storage(InPlace, Args && ... args) : object{forward<Args>(args)...} {}

    // Explicitly suppress calling member destructors.
    // The destructor below will do the right thing.
    ~Storage() {}
  } _storage;

  bool _full;

  constexpr MaybeBaseNT() : _storage{}, _full{false} {}
  constexpr ETL_IMPLICIT MaybeBaseNT(Nothing) : MaybeBaseNT{} {}

  template <typename ... Args>
  explicit MaybeBaseNT(InPlace, Args && ... args)
    : _storage{in_place, forward<Args>(args)...},
      _full{true} {}

  MaybeBaseNT(T const & value) : MaybeBaseNT{in_place, value} {}
  MaybeBaseNT(T && value) : MaybeBaseNT{in_place, move(value)} {}

  ~MaybeBaseNT() {
    if (_full) {
      _storage.object.~T();
    }
  }
};

/*
 * Base implementation of Maybe<T> for T with trivial destructor.
 *
 * This can act as a literal type when combined with a suitable T.
 */
template <typename T>
struct MaybeBaseT : MaybeTag {
  union Storage {
    struct {} dummy;
    T object;

    constexpr Storage() : dummy{} {}

    template <typename ... Args>
    constexpr Storage(InPlace, Args && ... args)
      : object{forward<Args>(args)...} {}
  } _storage;

  bool _full;

  constexpr MaybeBaseT() : _storage{}, _full{false} {}
  ETL_IMPLICIT constexpr MaybeBaseT(Nothing) : MaybeBaseT{} {}

  template <typename ... Args>
  constexpr explicit MaybeBaseT(InPlace, Args && ... args)
    : _storage{in_place, forward<Args>(args)...},
      _full{true} {}

  constexpr MaybeBaseT(T const & value)
    : MaybeBaseT{in_place, value} {}

  constexpr MaybeBaseT(T && value)
    : MaybeBaseT{in_place, move(value)} {}
};

/*
 * Selects between MaybeBaseT and MaybeBaseNT depending on whether the type
 * T has a non-trivial destructor.
 */
template <typename T>
using MaybeBase = ConditionalT<
  std::is_trivially_destructible<T>::value,
  MaybeBaseT<T>,
  MaybeBaseNT<T>>;


/*******************************************************************************
 * The actual Maybe template.
 */

template <typename T, typename Checking = LaxMaybeCheckPolicy>
class Maybe : private MaybeBase<T> {
  static_assert(!IsMaybe<T>::value, "Maybe<Maybe<T>> is not allowed.");
  static_assert(!std::is_same<T, Nothing>::value,
      "Maybe<Nothing> is not allowed.");

  using Base = MaybeBase<T>;
  using Base::_full;
  using Base::_storage;

public:
  /*****************************************************************
   * Construction, Destruction
   */

  // Inherit constructors appropriate to T.
  using Base::Base;

  /*
   * Copies a Maybe<T> when the types match exactly.
   *
   * This needs to be here to suppress default copy constructor generation,
   * even though the template version below *could* subsume it.
   */
  Maybe(Maybe<T, Checking> const & other) : MaybeBase<T>{} {
    if (other) in_place_construct(other.const_ref());
  }

  /*
   * Moves a Maybe<T> when the types match exactly.
   *
   * This needs to be here to suppress default copy constructor generation,
   * even though the template version below *could* subsume it.
   */
  Maybe(Maybe<T, Checking> && other) : MaybeBase<T>{} {
    if (other) in_place_construct(move(other.ref()));
  }

  /*
   * Copies a Maybe<S>, where T is constructible from S.
   */
  template <typename S, typename C>
  explicit Maybe(Maybe<S, C> const & other) : MaybeBase<T>{} {
    if (other) in_place_construct(other.const_ref());
  }

  /*
   * Moves a Maybe<S>, where T is constructible from S.
   */
  template <typename S, typename C>
  explicit Maybe(Maybe<S, C> && other) : MaybeBase<T>{} {
    if (other) in_place_construct(move(other.ref()));
  }

  /*
   * Destroys the Maybe<T>.  The real work is done in the superclass.
   */
  ~Maybe() = default;
 
  /*****************************************************************
   * Assignment
   */

  /*
   * Copy assignment.
   */
  template <typename S, typename C>
  Maybe &operator=(Maybe<S, C> const & other) {
    if (other) {
      if (!_full) in_place_construct(other.const_ref());
      else assign(other.const_ref());
    } else {
      clear();
    }
    return *this;
  }

  template <typename S, typename C>
  Maybe &operator=(Maybe<S, C> && other) {
    if (other) {
      if (!_full) in_place_construct(move(other.ref()));
      else assign(move(other.ref()));
    } else {
      clear();
    }
    return *this;
  }

  /*
   * Move/copy an S into the Maybe, causing it to become full.  T must be
   * constructible and assignable from S.  This implementation forwards to T's
   * move assignment operator, unless this Maybe is empty, in which case there
   * is no T on which to call the operator.  In that case we defer to the
   * constructor.
   */
  template <typename S>
  Maybe const & operator=(S && other) {
    if (_full) assign(forward<S>(other));
    else in_place_construct(forward<S>(other));
    return *this;
  }


  /*****************************************************************
   * Maybe-specific operations
   */

  /*
   * Destroys any contained value, leaving the Maybe empty.
   */
  void clear() {
    if (_full) {
      _storage.object.~T();
      _full = false;
    }
  }

  /*
   * Checks to see if this Maybe is something, rather than nothing.
   */
  constexpr bool is_something() const {
    return _full;
  }

  /*
   * Inverse of is_something
   */
  constexpr bool is_nothing() const {
    return !is_something();
  }

  /*
   * Returns a reference to the contained value, if one exists.
   *
   * Precondition: is_something()
   */
  constexpr T const & const_ref() const {
    return Checking::check_access(_full), _storage.object;
  }

  /*
   * Returns a reference to the contained value, if one exists.
   *
   * TODO(cbiffle): this can become constexpr in C++14.
   *
   * Precondition: is_something()
   */
  T & ref() {
    return Checking::check_access(_full), _storage.object;
  }

  /*****************************************************************
   * Sugar!
   */

  /*
   * Sugar to allow Maybe<T> to be tested in an explicit boolean context --
   * either a conversion like static_cast, or by a statement like if.
   *
   *   Maybe<X> foo;
   *   if (foo) {
   *     // => foo.is_something();
   *   } else {
   *     // => foo.is_nothing();
   *   }
   */
  constexpr explicit operator bool() const { return is_something(); }

  /*
   * Convenience syntax for clearing a Maybe by assignment from nothing.
   * This is exactly equivalent to the explicit clear function.
   */
  Maybe const & operator=(Nothing) {
    clear();
    return *this;
  }

private:
  template <typename ... Args>
  void in_place_construct(Args && ... args) {
    new (&_storage.object) T(forward<Args>(args)...);
    _full = true;
  }

  template <typename S>
  void assign(S && val) {
    _storage.object = forward<S>(val);
  }
};

/*
 * Comparisons between Maybes.  These kick in for any pair of types that have
 * defined comparison operators.  Note that we don't provide orderings, because
 * it's not obvious what the ordering between a user-defined type and nothing
 * would be.  Users can of course provide these if an ordering makes sense.
 *
 * Comparisons are optimistically constexpr.
 */
template <typename T1, typename C1, typename T2, typename C2>
constexpr bool operator==(Maybe<T1, C1> const &t, Maybe<T2, C2> const &s) {
  return (!bool(t) && !bool(s))
      || ((bool(t) && bool(s)) && t.const_ref() == s.const_ref());
}

template <typename T1, typename C1, typename T2, typename C2>
constexpr bool operator!=(Maybe<T1, C1> const &t, Maybe<T2, C2> const &s) {
  return (bool(t) != bool(s))
      || ((bool(t) && bool(s)) && t.const_ref() != s.const_ref());
}

/*
 * Comparisons between Maybe and non-maybe types.
 *
 * Optimistically constexpr.
 */
template <typename T1, typename C1, typename S>
constexpr bool operator==(Maybe<T1, C1> const &t, S const &s) {
  return bool(t) && t.const_ref() == s;
}

template <typename T1, typename C1, typename S>
constexpr bool operator==(S const & s, Maybe<T1, C1> const &t) {
  return bool(t) && s == t.const_ref();
}

template <typename T1, typename C1, typename S>
constexpr bool operator!=(Maybe<T1, C1> const &t, S const &s) {
  return bool(t) && t.const_ref() != s;
}

template <typename T1, typename C1, typename S>
constexpr bool operator!=(S const & s, Maybe<T1, C1> const &t) {
  return bool(t) && s != t.const_ref();
}

/*
 * Comparisons with Nothing.  We need this despite the implicit conversion
 * from Nothing -> Maybe<T> forall T, because implicit conversions don't kick
 * in during template argument matching for the templated operators above.
 *
 * On the plus side, we can confidently declare such operators constexpr.
 */
template <typename T, typename C>
constexpr bool operator==(Maybe<T, C> const &m, Nothing) {
  return m.is_nothing();
}

template <typename T, typename C>
constexpr bool operator==(Nothing, Maybe<T, C> const &m) {
  return m.is_nothing();
}

template <typename T, typename C>
constexpr bool operator!=(Maybe<T, C> const &m, Nothing) {
  return m.is_something();
}

template <typename T, typename C>
constexpr bool operator!=(Nothing, Maybe<T, C> const &m) {
  return m.is_something();
}

template <typename T, typename C = LaxMaybeCheckPolicy>
constexpr Maybe<T, C> just(T && arg) {
  return Maybe<T, C>(in_place, forward<T>(arg));
}


/*******************************************************************************
 * Maybe Check Policies
 */

struct LaxMaybeCheckPolicy {
  static constexpr bool check_access(bool) { return true; }
};

struct AssertMaybeCheckPolicy {
  static constexpr bool check_access(bool condition) {
    return ETL_ASSERT_CE(condition), true;
  }
};

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_MAYBE_H_INCLUDED
