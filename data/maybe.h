#ifndef ETL_DATA_MAYBE_H_
#define ETL_DATA_MAYBE_H_

#include "etl/common/attribute_macros.h"
#include "etl/common/utility.h"

namespace etl {
namespace data {

template <typename T>
class Maybe {
 public:
  constexpr ETL_INLINE Maybe() : _full(false) {}

  /*
   * Creates a full Maybe from any value that can be used to construct a T.
   */
  template <typename S>
  ETL_INLINE Maybe(S && value) : _value(etl::common::forward<S>(value)),
                             _full(true) {}

  /*
   * Copies a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  ETL_INLINE Maybe(Maybe<S> const & other) : _full(false) {
    if (other) {
      new(&_value) T(other._value);
      _full = true;
    }
  }

  /*
   * Moves a Maybe<S>, where T is constructible from S.
   */
  template <typename S>
  ETL_INLINE Maybe(Maybe<S> && other) : _full(false) {
    if (other) {
      new(&_value) T(etl::common::move(other._value));
      _full = true;
    }
  }

  ETL_INLINE ~Maybe() {
    if (_full) {
      _value.~T();
      _full = false;
    }
  }
 
  // Haven't needed this yet....
  Maybe &operator=(Maybe<T> const &other) = delete;

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
      _value = etl::common::move(other);
    } else {
      new(&_value) T(etl::common::forward<S>(other));
      _full = true;
    }

    return *this;
  }

  ETL_INLINE explicit constexpr operator bool() const { return _full; }

  ETL_INLINE T & operator *() {
    // TODO(cbiffle): assert.
    return _value;
  }

  ETL_INLINE T const & operator *() const {
    return _value;
  }

 private:
  union {
    T _value;
  };

  bool _full;
};

}  // namespace data
}  // namespace etl

#endif  // ETL_DATA_MAYBE_H_
