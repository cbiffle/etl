#ifndef ETL_ARMV7M_TYPES_H_
#define ETL_ARMV7M_TYPES_H_

#include "etl/common/types.h"

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

typedef etl::common::UInt64 DoubleWord;
typedef etl::common::UInt32 Word;
typedef etl::common::UInt16 HalfWord;
typedef etl::common::UInt8  Byte;

}  // namespace armv7m
}  // namespace etl

#endif  // ETL_ARMV7M_TYPES_H_
