#ifndef _ETL_MATH_FLOAT_H_INCLUDED
#define _ETL_MATH_FLOAT_H_INCLUDED

/*
 * Routines for manipulating floating-point numbers.
 */

#include <cstdint>

namespace etl {
namespace math {

/*
 * Sanity check.
 */
static_assert(sizeof(float) == sizeof(std::uint32_t),
    "This header assumes 32-bit 'float' type.");

using FloatFromBits = union { std::uint32_t i; float f; };
using FloatToBits = union { float f; std::uint32_t i; };

/*
 * Creates a single-precision float with the same bitwise representation as the
 * given integer.  Curiously missing from the C standard library, but analogous
 * to similar routines in every other language.
 */
constexpr inline float float_from_bits(std::uint32_t bits) {
  return FloatFromBits{bits}.f;
}

/*
 * Produces a 32-bit integer with the same bitwise representation as the given
 * single-precision float.  Curiously missing from the C standard library, but
 * analogous to similar routines in every other language.
 */
constexpr inline uint32_t float_to_bits(float f) {
  return FloatToBits{f}.i;
}

/*
 * Result type for fsplit, below.
 */
struct SplitFloat {
  float int_part;
  float frac_part;
};

/*
 * Separates a floating point number into its integral and fractional parts.
 * This is analogous to std::modf and follows its contract, but is made
 * significantly cheaper and somewhat more idiomatic by eliminating the pointer
 * out parameter.  This allows results to stay in registers for a ~20% gain.
 *
 * Note that this function can be made even cheaper (~50%) by inlining,
 * particularly into functions that are otherwise leaves.  If your compiler
 * supports link-time inlining decisions, you're likely golden.  If not (the
 * common case), you can manually use the inline version from inline_fsplit.h.
 */
SplitFloat fsplit(float);

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_FLOAT_H_INCLUDED
