#ifndef _ETL_BITS_H_INCLUDED
#define _ETL_BITS_H_INCLUDED

/*
 * Utilities for working with integers in terms of their sizes in bits.
 */

#include <cstdint>
#include <limits>
#include <type_traits>

#include "etl/type_list.h"
#include "etl/type_traits.h"

static_assert(std::numeric_limits<int>::radix == 2,
              "The routines below assume that integers are binary.");

namespace etl {

/*******************************************************************************
 * Detecting the number of bits in an integer type.
 */

/*
 * `etl::bit_width<T>()` (for some integer T) gives the type's width in bits.
 * It is nearly equivalent to `std::numeric_limits<T>::digits`, but with a
 * correction for the width of signed types.
 */
template <typename I>
static constexpr std::size_t bit_width() {
  // Apply a one-bit correction for the sign bit.
  return std::numeric_limits<I>::digits + std::is_signed<I>::value;
}

/*******************************************************************************
 * TypeLists wrapping some of the common integer type families.  Note that these
 * are sorted in ascending order of promised size.
 */

/*
 * Note: this list constraints this header to only working on systems that
 * provide at least this set of four exact-width integer types.  This may rule
 * out certain DSPs.  (Note that it does *not* rule out AVR.)
 */
using ExactInts = TypeList<
  std::int8_t,
  std::int16_t,
  std::int32_t,
  std::int64_t
>;

using LeastInts = TypeList<
  std::int_least8_t,
  std::int_least16_t,
  std::int_least32_t,
  std::int_least64_t
>;

using FastInts = TypeList<
  std::int_fast8_t,
  std::int_fast16_t,
  std::int_fast32_t,
  std::int_fast64_t
>;

using ExactUints = ExactInts::Map<std::make_unsigned>;
using LeastUints = LeastInts::Map<std::make_unsigned>;
using FastUints = FastInts::Map<std::make_unsigned>;


/*******************************************************************************
 * Selecting integer types by *exact* size in bits.
 */

template <std::size_t N>
struct IsExactInt {
  template <typename T>
  struct Predicate : BoolConstant<bit_width<T>() == N> {};
};

/*
 * Uint<N> is an unsigned integer type of exactly N bits.  This is a
 * compile-time-dynamic wrapper for the standard uintX_t types.
 */
template <std::size_t N>
using Uint = ExactUints::FindFirst<IsExactInt<N>::template Predicate>;

/*
 * Int<N> is a signed integer type of exactly N bits.  This is a
 * compile-time-dynamic wrapper for the standard intX_t types.
 */
template <std::size_t N>
using Int = ExactInts::FindFirst<IsExactInt<N>::template Predicate>;


/*******************************************************************************
 * Selecting integer types by *minimum* size in bits.
 */

template <std::size_t N>
struct IsLeastInt {
  template <typename T>
  struct Predicate : BoolConstant<bit_width<T>() >= N> {};
};

/*
 * UintLeast<N> is an unsigned integer type of at least N bits.  This is a
 * compile-time-dynamic wrapper for the standard uint_leastX_t types which
 * also supports non-power-of-two and non-multiple-of-eight counts.
 */
template <std::size_t N>
using UintLeast = LeastUints::FindFirst<IsLeastInt<N>::template Predicate>;

/*
 * IntLeast<N> is a signed integer type of at least N bits.  This is a
 * compile-time-dynamic wrapper for the standard int_leastX_t types which
 * also supports non-power-of-two and non-multiple-of-eight counts.
 */
template <std::size_t N>
using IntLeast = LeastInts::FindFirst<IsLeastInt<N>::template Predicate>;

/*
 * UintFast<N> is an unsigned integer type of at least N bits.  This is a
 * compile-time-dynamic wrapper for the standard uint_fastX_t types which
 * also supports non-power-of-two and non-multiple-of-eight counts.
 */
template <std::size_t N>
using UintFast = FastUints::FindFirst<IsLeastInt<N>::template Predicate>;

/*
 * IntFast<N> is a signed integer type of at least N bits.  This is a
 * compile-time-dynamic wrapper for the standard int_fastX_t types which
 * also supports non-power-of-two and non-multiple-of-eight counts.
 */
template <std::size_t N>
using IntFast = FastInts::FindFirst<IsLeastInt<N>::template Predicate>;


/*******************************************************************************
 * Calculating masks.
 */

template <std::size_t N, typename I = UintLeast<N>>
static constexpr I bit_mask() {
  static_assert(N <= bit_width<std::uintmax_t>(),
                "bit_mask size out of range for platform");

  return std::numeric_limits<I>::max() >> (bit_width<I>() - N);
}

}  // namespace etl

#endif  // _ETL_BITS_H_INCLUDED
