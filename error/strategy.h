#ifndef _ETL_ERROR_STRATEGY_H_INCLUDED
#define _ETL_ERROR_STRATEGY_H_INCLUDED

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

/*
 * Convenience implementation for the common case where an error is an
 * enumeration type with a "success value".
 */
template <typename E, E success>
struct TraditionalEnumStrategy {
  static constexpr bool is_error_type = true;
  static constexpr bool has_success_value = true;
  static constexpr E success_value = success;

  static constexpr bool is_bad(E error) {
    return error != success_value;
  }

  static constexpr E move_error(E error) { return error; }

  /*
   * This indicates that this type has no "success" component, causing
   * CHECK and friends to become void and preventing their use in expression
   * contexts.
   */
  static void move_success(E) {}
};

}  // namespace error
}  // namespace etl

#endif  // _ETL_ERROR_STRATEGY_H_INCLUDED
