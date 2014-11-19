#include "etl/data/crc32.h"

#include <array>
#include <cstddef>

#include "etl/integer_sequence.h"

using std::uint32_t;

namespace etl {
namespace data {


/*******************************************************************************
 * Table Generator
 *
 * We use the optimized CRC32 algorithm proposed in RFC1952.  It requires a
 * 256-entry lookup table,
 *
 * The original implementation allocates the lookup table lazily and calculates
 * its contents at runtime.  In our applications we can't afford such things.
 * Instead, we use constexpr to compute the lookup table at compile time and
 * toss it into ROM.
 *
 * This costs 4KiB of ROM (at least, when this feature is linked in).  It's
 * possible that we should eschew such extravagance and choose a slower
 * algorithm using a smaller table.  I'll revisit this later.
 */

/*
 * Here's a little function template for repeatedly applying a function object
 * to a seed value.  It's written to be tail-recursive, but I don't think any
 * compilers actually tail-call optimize constexpr evaluations.
 *
 * I'm not certain that it's useful enough to get lifted into a common library
 * header, but it *is* handy here.
 */
template <typename Fn, typename T>
static constexpr T iterate(unsigned n, Fn const & fn, T seed) {
  return (n <= 1) ? fn(seed) : iterate(n - 1, fn, fn(seed));
}

/*
 * The inner loop of the table generation process.  Updates the table entry with
 * a single data bit.  This is implemented as a function object because C++11
 * doesn't consider function references to be constexpr, even constexpr ones.
 */
struct Step {
  constexpr uint32_t operator()(uint32_t c) const {
    return (c & 1) ? (0xedb88320 ^ (c >> 1)) : (c >> 1);
  }
};

/*
 * Helper for generate, below.
 *
 * For some number of indices, generate the corresponding table entries as an
 * array.  This should normally be called only with an index sequence from 0 to
 * 255.
 *
 * This exists to deconstruct the IndexSequence produced in generate, and is the
 * sort of thing we must resort to in languages without pattern matching
 * *inside* of functions.
 */
template <std::size_t ... Is>
static constexpr auto generate_(etl::IndexSequence<Is...>)
    -> std::array<uint32_t, sizeof...(Is)> {
  return {{ iterate(8, Step{}, uint32_t(Is))... }};
}

/*
 * Generates a CRC32 lookup table of 256 entries, one for each possible byte of
 * input.
 */
static constexpr std::array<uint32_t, 256> generate() {
  return generate_(etl::MakeIndexSequence<256>{});
}


/*******************************************************************************
 * Runtime implementation
 */

/*
 * Our lookup table, marked constexpr to ensure that we're not doing this
 * during static construction.
 */
static constexpr auto crc32_table = generate();

/*
 * The CRC32 implementation.
 */
uint32_t crc32(RangePtr<std::uint8_t const> data, uint32_t seed) {
  uint32_t c = seed ^ 0xffffffff;
  for (auto byte : data) {
    c = crc32_table[(c ^ byte) & 0xff] ^ (c >> 8);
  }
  return c ^ 0xffffffff;
}

}  // namespace data
}  // namespace etl
