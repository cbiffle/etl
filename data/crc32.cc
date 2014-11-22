#include "etl/data/crc32.h"

#include <cstddef>

#include "etl/data/crc32_impl.h"

namespace etl {
namespace data {

#ifndef ETL_DATA_CRC32_TABLE_L2SIZE
#error To use this file you (or your build system) must define \
       ETL_DATA_CRC32_TABLE_L2SIZE.
#endif

static constexpr Crc32Table<ETL_DATA_CRC32_TABLE_L2SIZE> crc32_table {};

std::uint32_t crc32(RangePtr<std::uint8_t const> data, std::uint32_t seed) {
  return crc32_table.process(data, seed);
}

}  // namespace data
}  // namespace etl
