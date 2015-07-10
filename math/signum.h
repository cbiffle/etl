#ifndef _ETL_MATH_SIGNUM_H_INCLUDED
#define _ETL_MATH_SIGNUM_H_INCLUDED

namespace etl {
namespace math {

/*
 * The signum function from math:
 *          -1  if x < 0,
 * sgn(x) =  0  if x == 0,
 *          +1  if x > 0
 *
 * Curiously this function is missing from the C++ standard library.
 */
template <typename T>
constexpr int sgn(T value) {
  return (T{0} < value) - (value < T{0});
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_SIGNUM_H_INCLUDED
