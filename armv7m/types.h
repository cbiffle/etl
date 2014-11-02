#ifndef _ETL_ARMV7M_TYPES_H_INCLUDED
#define _ETL_ARMV7M_TYPES_H_INCLUDED

#include <cstdint>

namespace etl {
namespace armv7m {

/*
 * Abstract definitions of types used by the ARMv7-M architecture, using the
 * names used in the spec.  We use these in cases, like registers, where the
 * size is more important than the numeric interpretation.
 *
 * We happen to use unsigned integers below to avoid sign-extension surprises
 * when shifting.
 */

typedef std::uint64_t DoubleWord;
typedef std::uint32_t Word;
typedef std::uint16_t HalfWord;
typedef std::uint8_t  Byte;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_TYPES_H_INCLUDED
