#ifndef _ETL_ASSERT_H_INCLUDED
#define _ETL_ASSERT_H_INCLUDED

#include "etl/attribute_macros.h"

/*
 * An assertion facility similar to the standard's, but which can call through
 * to a portable assertion handler instead of doing whatever the libc feels
 * like doing (often involving stack unwinding and other things we don't
 * support).
 *
 * In general, ETL_ASSERT(foo) is equivalent to assert(foo).  If the macro
 * ETL_USE_SYSTEM_ASSERT is defined, the two are *precisely* equivalent.
 * Otherwise, ETL_ASSERT compiles code that tests the assertion; if it fails,
 * the hook function etl::assertion_failed is called.
 *
 * The application is responsible for defining etl::assertion_failed.
 *
 * The ETL implementation of ETL_ASSERT_CE is designed to be usable in constexpr
 * functions.  When ETL_USE_SYSTEM_ASSERT is defined, your mileage may vary.
 */

namespace etl {
  /*
   * Hook used by ETL_ASSERT when an assertion fails.  This is relevant
   * only when ETL_USE_SYSTEM_ASSERT is not defined, but it's prototyped
   * here unconditionally so that applications can change modes without
   * needing to compile out their implementations.
   *
   * Note that while assertion_failed has an argument slot to receive a
   * function name, ETL_ASSERT (below) does not pass one.  This is due to
   * an odd interpretation of constexpr semantics by the GCC team in 4.8/4.9.
   */
  ETL_NORETURN void assertion_failed(char const * file,
                                     int line,
                                     char const * function,
                                     char const * expression);
}  // namespace etl

#ifdef ETL_USE_SYSTEM_ASSERT

  #include <cassert>
  #define ETL_ASSERT(c) assert(c)
  #define ETL_ASSERT_CE(c) assert(c)

#else  // !defined(ETL_USE_SYSTEM_ASSERT)

  #define ETL_ASSERT(c) \
    ((c) ? void(0) : ::etl::assertion_failed(__FILE__, \
                                             __LINE__, \
                                             __func__, \
                                             #c))

  /*
   * __func__ is defined, in C99 and C++11, as equivalent to a static const
   * string at function scope.  In GCC 4.8/4.9, referencing it seems to cause
   * it to be added to the function...causing the function not to consist of
   * a return statement, and thus violate the constexpr rules for C++11.
   *
   * Sigh.
   *
   * Thus we provide a separate assert macro for use in constexpr environments.
   * TODO(cbiffle): unify these macros if the compiler folks figure out what
   * the right behavior is.
   */
  #define ETL_ASSERT_CE(c) \
    ((c) ? void(0) : ::etl::assertion_failed(__FILE__, \
                                             __LINE__, \
                                             "" /* TODO */, \
                                             #c))

#endif  // defined(ETL_USE_SYSTEM_ASSERT)


#endif  // _ETL_ASSERT_H_INCLUDED
