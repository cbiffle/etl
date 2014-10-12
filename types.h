#ifndef _ETL_TYPES_H_INCLUDED
#define _ETL_TYPES_H_INCLUDED

/*
 * Provides integer types with predictable sizes, independent of your choice
 * of compiler, processor, and ABI.
 *
 * For each of the traditional C integer widths (8, 16, 32, 64), this header
 * provides:
 *
 * - etl::IntX, a signed integer of X bits.
 * - etl::UIntX, an unsigned integer of X bits.
 *
 * In addition, there are integer types whose widths are defined in relation
 * to other aspects of the machine:
 * - etl::IntPtr is a signed integer wide enough to hold a pointer.
 * - etl::UIntPtr is an unsigned integer wide enough to hold a pointer.
 *
 * Analog of C++ <cstdint>.
 *
 * See also etl/size.h.
 */

#include "etl/type_traits.h"

namespace etl {

static_assert(etl::char_bits == 8,
              "ETL's explicitly-sized integer types can only be used on "
              "systems with 8-bit address units, because of a shortcut "
              "taken in the type selection algorithm.");

typedef typename SignedIntOfSize<1>::Type Int8;
typedef typename SignedIntOfSize<2>::Type Int16;
typedef typename SignedIntOfSize<4>::Type Int32;
typedef typename SignedIntOfSize<8>::Type Int64;

typedef typename SignedIntOfSize<sizeof(void *)>::Type IntPtr;

typedef typename UnsignedIntOfSize<1>::Type UInt8;
typedef typename UnsignedIntOfSize<2>::Type UInt16;
typedef typename UnsignedIntOfSize<4>::Type UInt32;
typedef typename UnsignedIntOfSize<8>::Type UInt64;

typedef typename UnsignedIntOfSize<sizeof(void *)>::Type UIntPtr;

}  // namespace etl

#endif  // _ETL_TYPES_H_INCLUDED
