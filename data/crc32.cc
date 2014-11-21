#include "etl/data/crc32.h"

#include <array>
#include <cstddef>

#include "etl/bits.h"
#include "etl/integer_sequence.h"

#include "etl/data/crc32_impl.h"

using std::uint32_t;

namespace etl {
namespace data {

static constexpr Crc32Table<4> crc32_table {};

uint32_t crc32(RangePtr<std::uint8_t const> data, uint32_t seed) {
  return crc32_table.process(data, seed);
}

}  // namespace data
}  // namespace etl
