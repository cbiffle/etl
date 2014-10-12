#ifndef _ETL_DATA_MAYBE_H_INCLUDED
#define _ETL_DATA_MAYBE_H_INCLUDED

#include "etl/attribute_macros.h"
#include "etl/implicit.h"
#include "etl/placement_new.h"
#include "etl/type_traits.h"
#include "etl/utility.h"

namespace etl {
namespace data {

/*
 * A Maybe<T> is a T-sized chunk of memory that may or may not contain useful
 * data.  It's intended as a replacement for passing around a (bool, address)
 * pair, just as RangePtr is intended as a replacement for (size, address)
 * pairs.  Like RangePtr, Maybe is designed to allow abstraction without
 * cost: it's designed to boil down to the code you would have written by
 * hand, with no penalty to code generation.
 *
 * (TODO(cbiffle): once I work out how to assert from here, this will become
 * conditionally true.)
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
 * This is a crapload of code for such a simple concept.  Perhaps it can be
 * simplified; I don't see an obvious way.  I've already used universal
 * references where possible to reduce duplication.  C++ sure loves boilerplate
 * and corner cases!
 */

struct Nothing {};
static Nothing constexpr nothing = {};

template <typename T> struct IsMaybe;

template <typename T>
class Maybe {
  template <typename S> friend class Maybe;
 public:
  /*
   * Attempting to use Maybe<Nothing> causes all sorts of problems.
   * This static_assert mostly serves to improve the error clarity.
   */
  static_assert(!etl::IsSame<Nothing, T>::value,
                "Maybe<Nothing> is an illegal type.");

  /*
   * Maybe<Maybe<...>> causes weird template match errors.
   */
  static_assert(!IsMaybe<T>::value, "Maybe cannot be nested.");

  /*****************************************************************
   * Construction, Destruction
   */

  /*
   * Creates an empty Maybe using the Nothing sentinel value.  Note that
   * this conversion is implicit.
   */
  ETL_INLINE ETL_IMPLICIT Maybe(Nothing) : _full(false) {}

  /*
   * Copies a Maybe<T> when the types match exactly.
   */
  ETL_INLINE Maybe(Maybe<T> const & other)
    : _full(other._full) {
    if (_full) new(&_value) T(other._value);
  }

  /*
   * Moves a Maybe<T> when the types match exactly.
   */
  ETL_INLINE Maybe(Maybe<T> && other) : _full(other._full) {
    if (_full) {
      new(&_value) T(::etl::move(other._value));
      other._full = false;
    }
  }

  /*
   * Creates a full Maybe by universal reference to any type S assignable to T.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(S && value,
      typename ::etl::EnableIf<!IsMaybe<S>::value, void *>::Type = 0)
    : _value(etl::forward<S>(value)),
      _full(true) {}

  /*
   * Copies a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(Maybe<S> const & other,
      typename ::etl::EnableIf<!::etl::IsSame<T, S>::value, void *>::Type = 0)
    : _full(other._full) {
    if (_full) new(&_value) T(other._value);
  }

  /*
   * Moves a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(Maybe<S> && other) : _full(other._full) {
    if (_full) {
      new(&_value) T(etl::move(other._value));
      other._full = false;
    }
  }

  ETL_INLINE ~Maybe() {
    clear();
  }
 
  /*****************************************************************
   * Assignment
   */

  /*
   * Copy assignment.
   */
  template <typename S>
  Maybe &operator=(Maybe<S> const & other) {
    if (other) {
      if (_full) {
        _value = other._value;
      } else {
        new(&_value) T(other._value);
        _full = true;
      }
    } else {
      clear();
    }
    return *this;
  }

  template <typename S>
  Maybe &operator=(Maybe<S> && other) {
    if (other) {
      if (_full) {
        _value = etl::move(other._value);
      } else {
        new(&_value) T(etl::move(other._value));
        _full = true;
      }
    } else {
      clear();
    }
    return *this;
  }

  /*
   * Move/copy an S into the Maybe, causing it to become full.  T must be
   * constructible from S.  This implementation forwards to T's
   * move assignment operator, unless this Maybe is empty, in which
   * case there is no T on which to call the operator.  In that case
   * we defer to the constructor.
   */
  template <typename S>
  ETL_INLINE Maybe const & operator=(S && other) {
    if (_full) {
      _value = etl::forward<S>(other);
    } else {
      new(&_value) T(etl::forward<S>(other));
      _full = true;
    }

    return *this;
  }


  /*****************************************************************
   * Maybe-specific operations
   */

  /*
   * Destroys any contained value, leaving the Maybe empty.
   */
  ETL_INLINE void clear() {
    if (_full) {
      _value.~T();
      _full = false;
    }
  }

  /*
   * Checks to see if this Maybe is something, rather than nothing.
   */
  ETL_INLINE bool is_something() const {
    return _full;
  }

  /*
   * Inverse of is_something
   */
  ETL_INLINE bool is_nothing() const {
    return !is_something();
  }

  /*
   * Returns a reference to the contained value, if one exists.
   *
   * Precondition: is_something()
   */
  ETL_INLINE T const & const_ref() const {
    // TODO(cbiffle): assert
    return _value;
  }

  /*
   * Returns a reference to the contained value, if one exists.
   *
   * Precondition: is_something()
   */
  ETL_INLINE T & ref() {
    // TODO(cbiffle): assert
    return _value;
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
  ETL_INLINE explicit operator bool() const { return is_something(); }

  /*
   * Convenience syntax for clearing a Maybe by assignment from nothing.
   * This is exactly equivalent to the explicit clear function.
   */
  ETL_INLINE Maybe const & operator=(Nothing) {
    clear();
    return *this;
  }

 private:
  union {
    T _value;
  };

  bool _full;
};

/*
 * Comparisons between Maybes.  These kick in for any pair of types that have
 * defined comparison operators.  Note that we don't provide orderings, because
 * it's not obvious what the ordering between a user-defined type and nothing
 * would be.  Users can of course provide these if an ordering makes sense.
 */
template <typename T, typename S>
bool operator==(Maybe<T> const &t, Maybe<S> const &s) {
  bool t_ = t, s_ = s;
  return (!t_ && !s_) || ((t_ && s_) && t.const_ref() == s.const_ref());
}

template <typename T, typename S>
bool operator!=(Maybe<T> const &t, Maybe<S> const &s) {
  bool t_ = t, s_ = s;
  return (t_ != s_) || ((t_ && s_) && t.const_ref() != s.const_ref());
}

/*
 * Comparisons with Nothing.  We need this despite the implicit conversion
 * from Nothing -> Maybe<T> forall T, because implicit conversions don't kick
 * in during template argument matching for the templated operators above.
 */
template <typename T>
bool operator==(Maybe<T> const &m, Nothing) {
  return m.is_nothing();
}

template <typename T>
bool operator==(Nothing, Maybe<T> const &m) {
  return m.is_nothing();
}

template <typename T>
bool operator!=(Maybe<T> const &m, Nothing) {
  return m.is_something();
}

template <typename T>
bool operator!=(Nothing, Maybe<T> const &m) {
  return m.is_something();
}


template <typename T>
struct IsRawMaybe : public ::etl::BoolConstant<false> {};

template <typename T>
struct IsRawMaybe<Maybe<T>> : public ::etl::BoolConstant<true> {};

template <typename T>
struct IsMaybe {
private:
  typedef typename ::etl::RemoveReference<T>::Type Tnoref;
  typedef typename ::etl::RemoveQualifiers<Tnoref>::Type Traw;

public:
  static constexpr bool value = IsRawMaybe<Traw>::value;
};

template <typename T>
Maybe<T> just(T && value) {
  return Maybe<T>(etl::forward<T>(value));
}

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_MAYBE_H_INCLUDED
