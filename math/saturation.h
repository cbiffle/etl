#ifndef _ETL_MATH_SATURATION_H_INCLUDED
#define _ETL_MATH_SATURATION_H_INCLUDED

#include <type_traits>

namespace etl {
namespace math {

template <typename T>
constexpr T clamp(T value, T min, T max) {
  return (value < min) ? min : (value > max) ? max : value;
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_SATURATION_H_INCLUDED
