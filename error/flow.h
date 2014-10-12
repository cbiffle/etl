#ifndef _ETL_ERROR_FLOW_H_INCLUDED
#define _ETL_ERROR_FLOW_H_INCLUDED

/*
 * These macros provide an alternative to exceptions for non-local flow
 * control when errors are encountered.
 *
 * A common C/C++ error handling idiom reads as follows:
 *
 *   int e;
 *   e = operation1();
 *   if (e != OK) return e;
 *   e = operation2();
 *   if (e != OK) return e;
 *   ... and so on
 *
 * (In C code, or C++ code that can't rely on RAII patterns, the 'return e'
 * may be replaced by 'goto label'.)
 *
 * This isn't a great pattern for several reasons:
 * - It encourages large-scoped variables, with their attendant perils.
 * - It's verbose.
 * - It's repetitive, in that most dangerous way where a single *incorrect*
 *   repetition is both easy to miss and potentially fatal (as in the 'goto
 *   fail' bug).
 *
 * The equivalent code written with the ETL error flow macros reads:
 *
 *   ETL_CHECK(operation1());
 *   ETL_CHECK(operation2());
 *
 * The macros generate code that includes a return statement.  The reader can
 * concentrate on the straight-line non-error execution path, but is made
 * aware of the precise points where the function may terminate early (the
 * macro uses).
 *
 * If you really want to get the error into a variable, without affecting
 * control flow, there is ETL_CATCH.  It's most useful with more complex
 * error types and is rare.
 *
 * Finally, there's ETL_IGNORE, which explicitly does not act on the success
 * or failure of an operation.
 */

#include "etl/error/strategy.h"

#define ETL_FLOW_PRECONDITION(macro, expr) \
  static_assert(::etl::error::Strategy<decltype(expr)>::is_error_type, \
                "Only types that participate in the ETL error handling " \
                "system can be used with " #macro ".  The result of this " \
                "expression does not appear to participate: " \
                #expr) \

#define ETL_CHECK(expr) \
  ({ \
    ETL_FLOW_PRECONDITION(CHECK, expr); \
    \
    decltype(expr) etl_flow_value(expr); \
    if (::etl::error::Strategy<decltype(expr)>::is_bad(etl_flow_value)) { \
      return ::etl::error::Strategy<decltype(expr)> \
          ::move_error(etl_flow_value); \
    } \
    ::etl::error::Strategy<decltype(expr)>::move_success(etl_flow_value); \
  })

#define ETL_CATCH(expr) \
  ({ \
    ETL_FLOW_PRECONDITION(CATCH, expr); \
    \
    decltype(expr) etl_flow_value(expr); \
    ::etl::error::Strategy<decltype(expr)>::move_error(etl_flow_value); \
  })

#define ETL_IGNORE(expr) \
  ({ \
    ETL_FLOW_PRECONDITION(IGNORE, expr); \
    \
    decltype(expr) etl_flow_value(expr); \
    static_cast<void>(etl_flow_value); \
  })

#endif  // _ETL_ERROR_FLOW_H_INCLUDED
