#include "etl/math/trig.h"

#include <array>
#include <cmath>
#include <utility>

#include "etl/assert.h"
#include "etl/integer_sequence.h"
#include "etl/prediction.h"

#include "etl/math/inline_fsplit.h"

#ifndef ETL_CONFIG_USE_TOOLCHAIN_TRIG

namespace etl {
namespace math {

/*
 * Implementation factor of sin and cos.  Potentially generic.
 *
 * - 'angle' gives an angle in radians.  Its domain is not bounded, but values
 *   between 0 and 2pi will get best precision.
 * - 'negate' gives the factor to apply when the angle is negative -- -1 for
 *   sine, 1 for cosine.
 * - 'points' is the number of points in the lookup table.
 * - 'table' is the base address of the lookup table.
 *
 * Note: you might be inclined to change the namespaces below to structs and
 * templatize this operation on the struct type.  I certainly was.  But GCC
 * 4.8.3 (at least) has a hard time with the constexpr table generation if it
 * isn't at namespace scope.  So it goes.
 */
template <int negate, unsigned l2points>
static inline float lookup_and_interpolate(float angle,
                                           __fp16 const * table) {
  if (angle < 0) {
    return negate * lookup_and_interpolate<negate, l2points>(-angle, table);
  }

  auto domain = (angle / (2 * float(M_PI))) * (1u << l2points);
  auto parts = fsplit_inl(domain);

  auto index0 = (unsigned(parts.int_part)) % (1u << l2points);
  auto index1 = (index0 + 1) % (1u << l2points);

  auto base = table[index0];
  auto slope = table[index1] - base;

  return base + slope * parts.frac_part;
}


/*
 * Sine.
 */

namespace _sin {
  static constexpr unsigned l2points = 9;
  static constexpr unsigned points = 1u << l2points;

  static constexpr int negate = -1;

  template <std::size_t ... is>
  static constexpr std::array<__fp16, sizeof...(is)>
      gen_(IndexSequence<is...>) {
    return {{ __fp16(std::sin(float(is) * (2 * float(M_PI)) / points))... }};
  }

  static constexpr std::array<__fp16, points> gen() {
    return gen_(MakeIndexSequence<points>{});
  }

  __attribute__((section(".ramcode.math.sin_table")))
  static constexpr std::array<__fp16, points> table = gen();
};

float sin(float angle) {
  return lookup_and_interpolate<_sin::negate, _sin::l2points>(angle,
                                                              &_sin::table[0]);
}


/*
 * Cosine.
 */

namespace _cos {
  static constexpr unsigned l2points = 9;
  static constexpr unsigned points = 1u << l2points;

  static constexpr int negate = 1;

  template <std::size_t ... is>
  static constexpr std::array<__fp16, sizeof...(is)>
      gen_(IndexSequence<is...>) {
    return {{ __fp16(std::cos(float(is) * (2 * float(M_PI)) / points))... }};
  }

  static constexpr std::array<__fp16, points> gen() {
    return gen_(MakeIndexSequence<points>{});
  }

  __attribute__((section(".ramcode.math.cos_table")))
  static constexpr std::array<__fp16, points> table = gen();
};

float cos(float angle) {
  return lookup_and_interpolate<_cos::negate, _cos::l2points>(angle,
                                                              &_cos::table[0]);
}

}  // namespace math
}  // namespace etl

#endif  // defined(ETL_CONFIG_USE_TOOLCHAIN_TRIG)
