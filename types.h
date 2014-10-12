#ifndef ETL_TYPES_H_
#define ETL_TYPES_H_

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

#define ETL_SIZED_INT(n, name) \
  typedef typename SelectBySize<n, signed char, short, int, long, long long>\
                       ::Type name

ETL_SIZED_INT(1, Int8);
ETL_SIZED_INT(2, Int16);
ETL_SIZED_INT(4, Int32);
ETL_SIZED_INT(8, Int64);

ETL_SIZED_INT(sizeof(void *), IntPtr);

#undef ETL_SIZED_INT

#define ETL_UNSIGNED_INT(n) \
  typedef typename MakeUnsigned<Int ## n>::Type UInt ## n

ETL_UNSIGNED_INT(8);
ETL_UNSIGNED_INT(16);
ETL_UNSIGNED_INT(32);
ETL_UNSIGNED_INT(64);

#undef ETL_UNSIGNED_INT

typedef typename MakeUnsigned<IntPtr>::Type UIntPtr;

}  // namespace etl

#endif  // ETL_TYPES_H_
