#ifndef _ETL_ALGORITHM_H_INCLUDED
#define _ETL_ALGORITHM_H_INCLUDED

/**
 * @addtogroup std
 * @{
 */

/**
 * @file
 *
 * Provides algorithmic utility functions that weren't interesting enough
 * to merit their own header.
 *
 * Analog of C++ `<algorithm>`, which is not available in freestanding
 * implementations.
 */

namespace etl {

/**
 * Returns the lesser of `a` and `b`.
 */
template <typename T>
constexpr T const & min(T const & a, T const & b) {
  return a < b ? a : b;
}

/**
 * Returns the greater of `a` and `b`.
 */
template <typename T>
constexpr T const & max(T const & a, T const & b) {
  return a > b ? a : b;
}

}  // namespace etl

/**@}*/

#endif  // _ETL_ALGORITHM_H_INCLUDED
