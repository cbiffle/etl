#ifndef _ETL_ERROR_RESULT_H_INCLUDED
#define _ETL_ERROR_RESULT_H_INCLUDED

#include "etl/assert.h"
#include "etl/utility.h"

#include "etl/error/strategy.h"

namespace etl {
namespace error {

/*
 * Tag type used to signify an error result.
 */
struct Left {};
static constexpr Left left {};

/*
 * Tag type used to signify a success result.
 */
struct Right {};
static constexpr Right right {};

/*
 * Return value from operations that may fail; represents a return code with an
 * optional result value that is only constructed if the return code signifies
 * success.
 */
template <typename E, typename V>
class Result {
public:
  using EStrategy = Strategy<E>;

  static_assert(EStrategy::is_error_type,
                "Result must be used with participating error types");

  Result() = delete;

  constexpr Result(E const & error)
    : _status{(ETL_ASSERT_CE(EStrategy::is_bad(error)), error)} {}

  constexpr Result(Left const &, E const & error)
    : _status{(ETL_ASSERT_CE(EStrategy::is_bad(error)), error)} {}

  template <typename W>
  Result(Right const &, W && value)
    : _status{EStrategy::success_value},
      _storage{etl::forward<W>(value)} {}

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
    return _storage.object;
  }

private:
  E _status;

  union Storage {
    struct {} dummy;
    V object;

    ETL_INLINE constexpr Storage() : dummy{} {}

    template <typename W>
    ETL_INLINE Storage(W && value) : object{forward<W>(value)} {}
  } _storage;
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
