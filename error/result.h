#ifndef _ETL_ERROR_RESULT_H_INCLUDED
#define _ETL_ERROR_RESULT_H_INCLUDED

#include "etl/assert.h"
#include "etl/utility.h"

#include "etl/error/strategy.h"

namespace etl {
namespace error {

struct Left {};
static constexpr Left left {};

struct Right {};
static constexpr Right right {};

template <typename E, typename V>
class Result {
public:
  using EStrategy = Strategy<E>;

  static_assert(EStrategy::is_error_type,
                "Result must be used with participating error types");

  Result() = delete;

  constexpr Result(E const & error)
    : _status((ETL_ASSERT_CE(EStrategy::is_bad(error)), error)) {}

  constexpr Result(Left const &, E const & error)
    : _status((ETL_ASSERT_CE(EStrategy::is_bad(error)), error)) {}

  template <typename W>
  Result(Right const &, W && value)
    : _status(EStrategy::success_value),
      _value(etl::forward<W>(value)) {}

  constexpr bool is_error() const {
    return EStrategy::is_bad(_status);
  }

  constexpr explicit operator bool() const {
    return !is_error();
  }

  constexpr E get_status() const {
    return _status;
  }

  V & ref() {
    ETL_ASSERT(!is_error());
    return _value;
  }

private:
  E _status;
  union {
    V _value;
  };
};

template <typename E, typename V>
struct Strategy<Result<E, V>> {
  static constexpr bool is_error_type = true;

  static constexpr bool has_success_value = false;

  static bool is_bad(Result<E, V> const & e) {
    return e.is_error();
  }

  static E move_error(Result<E, V> & e) {
    return etl::move(e.get_status());
  }

  static V move_success(Result<E, V> & e) {
    return etl::move(e.ref());
  }
};

}  // namespace error
}  // namespace etl

#endif  // _ETL_ERROR_RESULT_H_INCLUDED
