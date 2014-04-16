#ifndef ETL_TYPES_H_
#define ETL_TYPES_H_

#include "etl/type_traits.h"

namespace etl {

#define ETL_SIZED_INT(n, name) \
  typedef typename SelectBySize<n, signed char, short, int, long, long long>\
                       ::Type name

ETL_SIZED_INT(1, Int8);
ETL_SIZED_INT(2, Int16);
ETL_SIZED_INT(4, Int32);
ETL_SIZED_INT(8, Int64);

#undef ETL_SIZED_INT

#define ETL_UNSIGNED_INT(n) \
  typedef typename MakeUnsigned<Int ## n>::Type UInt ## n

ETL_UNSIGNED_INT(8);
ETL_UNSIGNED_INT(16);
ETL_UNSIGNED_INT(32);
ETL_UNSIGNED_INT(64);

#undef ETL_UNSIGNED_INT

typedef typename SelectBySize<sizeof(void *),
                              signed char, short, int, long, long long>::Type
        IntPtr;

typedef typename MakeUnsigned<IntPtr>::Type UIntPtr;

}  // namespace etl

#endif  // ETL_TYPES_H_
