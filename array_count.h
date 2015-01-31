#ifndef _ETL_ARRAY_COUNT_H_INCLUDED
#define _ETL_ARRAY_COUNT_H_INCLUDED

#include <cstddef>

/*
 * Finds the number of elements in a static array.  Replacement for the error
 * prone C idiom "sizeof(e) / sizeof(e[0])".
 */

namespace etl {

template <typename E, std::size_t count>
constexpr std::size_t array_count(E (&)[count]) {
  return count;
}

}  // namespace etl

#endif  // _ETL_ARRAY_COUNT_H_INCLUDED
