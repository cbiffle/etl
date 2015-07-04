#ifndef _ETL_MATH_TRIG_H_INCLUDED
#define _ETL_MATH_TRIG_H_INCLUDED

/*
 * Lookup-table-based trigonometric functions.
 *
 * These are significantly faster, albeit somewhat less accurate, than the
 * GCC toolchain routines.  If accuracy is more important than speed in your
 * application, define ETL_CONFIG_USE_TOOLCHAIN_TRIG in your build environment
 * (using Cobble, set etl_config_use_toolchain_trig = True).
 *
 * The implementation of sin and cos currently relies on compiler support for
 * half-precision floating point, to keep the tables compact.  Such support is
 * unavailable on all but the latest Intel processors, so you may need to
 * disable ETL's trig on such machines.
 */

#ifdef ETL_CONFIG_USE_TOOLCHAIN_TRIG

#include <cmath>

namespace etl {
namespace math {

/*
 * Import the names from the standard library.
 */
using std::sin;
using std::cos;

}  // namespace math
}  // namespace etl

#else  // !defined(ETL_CONFIG_USE_TOOLCHAIN_TRIG)

/*
 * Provide prototypes for our own trig implementation.
 */

namespace etl {
namespace math {

__attribute__((const))
float sin(float);

__attribute__((const))
float cos(float);

}  // namespace math
}  // namespace etl

#endif


#endif  // _ETL_MATH_TRIG_H_INCLUDED
