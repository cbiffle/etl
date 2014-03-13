#ifndef ETL_DATA_MAYBE_H_
#define ETL_DATA_MAYBE_H_

#include "etl/common/attribute_macros.h"
#include "etl/common/utility.h"

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
 */

struct Nothing {};
static Nothing constexpr nothing = {};

template <typename T>
class Maybe {
 public:
  /*
   * Attempting to use Maybe<Nothing> causes all sorts of problems.
   * This static_assert mostly serves to improve the error clarity.
   */
  static_assert(!etl::common::IsSame<Nothing, T>::value,
                "Maybe<Nothing> is an illegal type.");


  /*****************************************************************
   * Construction, Destruction
   */

  /*
   * Creates an empty Maybe using the Nothing sentinel value.  Note that
   * this conversion is implicit.
   */
  ETL_INLINE constexpr Maybe(Nothing) : _full(false) {}

  /*
   * Creates a full Maybe from any value that can be used to construct a T.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(S && value)
    : _value(etl::common::forward<S>(value)),
      _full(true) {}

  /*
   * Copies a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(Maybe<S> const & other) : _full(false) {
    if (other) {
      new(&_value) T(other._value);
      _full = true;
    }
  }

  /*
   * Moves a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  explicit ETL_INLINE Maybe(Maybe<S> && other) : _full(false) {
    if (other) {
      new(&_value) T(etl::common::move(other._value));
      _full = true;
    }
  }

  ETL_INLINE ~Maybe() {
    clear();
  }
 
  /*****************************************************************
   * Assignment
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
        _value = etl::common::move(other._value);
      } else {
        new(&_value) T(etl::common::move(other._value));
        _full = true;
      }
    } else {
      clear();
    }
    return *this;
  }

  /*
   * Copy an S into the Maybe, causing it to become full.  T must be
   * constructible from S.  This implementation forwards to T's
   * copy assignment operator, unless this Maybe is empty, in which
   * case there is no T on which to call the operator.  In that case
   * we defer to the constructor.
   */
  template <typename S>
  ETL_INLINE Maybe const & operator=(S const & other) {
    if (_full) {
      _value = other;
    } else {
      new(&_value) T(other);
      _full = true;
    }
    return *this;
  }

  /*
   * Move an S into the Maybe, causing it to become full.  T must be
   * constructible from S.  This implementation forwards to T's
   * move assignment operator, unless this Maybe is empty, in which
   * case there is no T on which to call the operator.  In that case
   * we defer to the constructor.
   */
  template <typename S>
  ETL_INLINE Maybe const & operator=(S && other) {
    if (_full) {
      _value = etl::common::forward<S>(other);
    } else {
      new(&_value) T(etl::common::forward<S>(other));
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
  ETL_INLINE constexpr bool is_something() const {
    return _full;
  }

  /*
   * Inverse of is_something
   */
  ETL_INLINE constexpr bool is_nothing() const {
    return !is_something();
  }

  /*
   * Returns a reference to the contained value, if one exists.
   *
   * Precondition: is_something()
   */
  ETL_INLINE constexpr T const & const_ref() const {
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
  ETL_INLINE explicit constexpr operator bool() const { return is_something(); }

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
 * defined comparison operators.
 */
#define ETL_DATA_MAYBE_COMPARISON(op) \
  template <typename T, typename S> \
  bool operator op(Maybe<T> const &t, Maybe<S> const &s) { \
    return (t && s) && t.Get() op s.Get(); \
  }

ETL_DATA_MAYBE_COMPARISON(==)
ETL_DATA_MAYBE_COMPARISON(!=)
ETL_DATA_MAYBE_COMPARISON(<)
ETL_DATA_MAYBE_COMPARISON(>)
ETL_DATA_MAYBE_COMPARISON(<=)
ETL_DATA_MAYBE_COMPARISON(>=)

#undef ETL_DATA_MAYBE_COMPARISON

}  // namespace data
}  // namespace etl

#endif  // ETL_DATA_MAYBE_H_
