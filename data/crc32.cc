#include "etl/data/crc32.h"

#include <array>
#include <cstddef>

#include "etl/integer_sequence.h"

using std::uint32_t;

namespace etl {
namespace data {

/*
 * The inner loop of the table generation process.  Updates the table entry with
 * a single data bit.
 */
static constexpr uint32_t step(uint32_t c) {
  return (c & 1) ? (0xedb88320 ^ (c >> 1)) : (c >> 1);
}

/*
 * Hand-unrolled 8-step process for generating a single table entry.
 */
static constexpr uint32_t step8(uint32_t c) {
  return step(step(step(step(step(step(step(step(c))))))));
}

/*
 * Helper for generate, below.
 *
 * For some number of indices, generate the corresponding table entries as an
 * array.  This should normally be called only with an index sequence from 0 to
 * 255.
 */
template <std::size_t ... Is>
static constexpr auto generate_(etl::IndexSequence<Is...>)
    -> std::array<uint32_t, sizeof...(Is)> {
  return {{ step8(Is)... }};
}

/*
 * Generates a CRC32 lookup table of 256 entries, one for each possible byte of
 * input.
 */
static constexpr std::array<uint32_t, 256> generate() {
  return generate_(etl::MakeIndexSequence<256>{});
}

static constexpr auto crc32_table = generate();

uint32_t crc32(RangePtr<std::uint8_t const> data, uint32_t seed) {
  uint32_t c = seed ^ 0xffffffff;
  for (std::uint8_t byte : data) {
    c = crc32_table[(c ^ byte) & 0xff] ^ (c >> 8);
  }
  return c ^ 0xffffffff;
}

}  // namespace data
}  // namespace etl
