#ifndef ETL_ARMV7M_TYPES_H_
#define ETL_ARMV7M_TYPES_H_

namespace etl {
namespace armv7m {

/*
 * Abstract definitions of types used by the ARMv7-M architecture, using the
 * names used in the spec.  We use these in cases, like registers, where the
 * size is more important than the numeric interpretation.
 *
 * We happen to use unsigned integers below to avoid sign-extension surprises
 * when shifting.
 *
 * Note that we don't use stdint-style explicit width integers.  This is to
 * allow this file to be used by programs that don't use any C library.  The
 * sizes of the types below are specified by the ARM C ABI, so it's okay.
 */

typedef unsigned long long DoubleWord;
typedef unsigned long      Word;
typedef unsigned short     HalfWord;
typedef unsigned char      Byte;

}  // namespace armv7m
}  // namespace etl

#endif  // ETL_ARMV7M_TYPES_H_
