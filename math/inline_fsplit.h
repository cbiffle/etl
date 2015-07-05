#ifndef _ETL_MATH_INLINE_FSPLIT_H_INCLUDED
#define _ETL_MATH_INLINE_FSPLIT_H_INCLUDED

#include "etl/math/float.h"

namespace etl {
namespace math {

/*
 * Inline version of fsplit.  Analogous to std::modf and follows its contract.
 * See float.h for more details.
 */
inline SplitFloat fsplit_inl(float const value) {
  auto const bits = math::float_to_bits(value);
  auto const exp = int32_t((bits >> 23) & 0xFF) - 127;
  auto const sign = math::float_from_bits(bits & (1u << 31));

  float i_part = value;
  float f_part = sign;

  if (ETL_UNLIKELY(exp > 22)) {
    // There are no fractional bits.
  } else if (ETL_UNLIKELY(exp < 0)) {
    // There are no integral bits.
    i_part = sign;
    f_part = value;
  } else {
    // There are some of each, though the fractional bits might be zero.
    auto const frac_mask = ((1u << 23) - 1) >> exp;
    if (bits & frac_mask) {
      // Fractional bits aren't zero, we can do this the easy way:
      i_part = math::float_from_bits(bits & ~frac_mask);
      f_part = value - i_part;
    } else {
      // Fractional bits are zero; if we subtract the integral part as above,
      // we'll get the sign of the fractional result wrong.
      // Just treat this as having no fractional bits.
    }
  }

  return {i_part, f_part};
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_INLINE_FSPLIT_H_INCLUDED
