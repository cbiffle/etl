#ifndef _ETL_DATA_CRC32_H_INCLUDED
#define _ETL_DATA_CRC32_H_INCLUDED

#include <cstdint>

#include "etl/data/range_ptr.h"

namespace etl {
namespace data {

/*
 * An implementation of the popular CRC-32 algorithm, as specified in RFC1952
 * among other places.
 *
 * To be clear, this is the algorithm with polynomial 0xEDB88320 as used by
 * Gzip, Ethernet, SATA, and others.
 *
 * The CRCs of multiple blocks can be chained by passing the result of one call
 * as the 'seed' of the next.
 *
 * The implementation involves a time-space tradeoff.  You can choose two
 * different optimization strategies for the implementation by depending on
 * either //etl/data:crc32_fast or //etl/data:crc32_small.  Alternatively, you
 * can adjust the parameters yourself by using crc32_impl.h instead.
 */
std::uint32_t crc32(RangePtr<std::uint8_t const>, std::uint32_t seed = 0);

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_CRC32_H_INCLUDED
