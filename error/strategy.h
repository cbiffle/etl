#ifndef ETL_ERROR_STRATEGY_H_
#define ETL_ERROR_STRATEGY_H_

namespace etl {
namespace error {

/*
 * Describes how a type participates in the error handling system.
 *
 * By default, types don't participate.  Types can be opted in by specializing
 * Strategy.  This allows a type to be used with CHECK and the other error
 * handling facilities.
 *
 * Error types are classified into two groups: *basic errors* carry only an
 * error/success indicator, while *wrapped errors* carry an arbitrary value
 * on success.
 */
template <typename E>
struct Strategy {
  static constexpr bool is_error_type = false;
};

}  // namespace error
}  // namespace etl

#endif  // ETL_ERROR_STRATEGY_H_
