#ifndef _ETL_ALGORITHM_H_INCLUDED
#define _ETL_ALGORITHM_H_INCLUDED

/*
 * Provides algorithmic utility functions that weren't interesting enough
 * to merit their own header.
 *
 * Analog of C++ <algorithm>, which is not available in freestanding
 * implementations.
 */

namespace etl {

template <typename T>
constexpr T const & min(T const & a, T const & b) {
  return a < b ? a : b;
}

template <typename T>
constexpr T const & max(T const & a, T const & b) {
  return a > b ? a : b;
}

}  // namespace etl

#endif  // _ETL_ALGORITHM_H_INCLUDED
