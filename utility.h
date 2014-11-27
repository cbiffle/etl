#ifndef _ETL_UTILITY_H_INCLUDED
#define _ETL_UTILITY_H_INCLUDED

/*
 * Type and scope manipulation operators that weren't interesting enough
 * to merit their own headers.
 *
 * Analog of C++ <utility>, which is not available in a freestanding
 * implementation (see discussion in n3256).
 */

#include <type_traits>

#include "etl/attribute_macros.h"

namespace etl {

template <typename T>
ETL_INLINE
constexpr T && forward(typename std::remove_reference<T>::type & value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE
constexpr T && forward(typename std::remove_reference<T>::type && value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE
constexpr typename std::remove_reference<T>::type && move(T && value) {
  return static_cast<typename std::remove_reference<T>::type &&>(value);
}

template <typename T>
void swap(T & a, T & b) {
  T intermediate = move(a);
  a = move(b);
  b = move(intermediate);
}

template <typename T, std::size_t count>
void swap(T (&a)[count], T (&b)[count]) {
  for (std::size_t i = 0; i < count; ++i) {
    swap(a[i], b[i]);
  }
}

}  // namespace etl

#endif  // _ETL_UTILITY_H_INCLUDED
