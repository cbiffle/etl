#ifndef ETL_ERROR_FLOW_H_
#define ETL_ERROR_FLOW_H_

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

#endif  // ETL_ERROR_FLOW_H_
