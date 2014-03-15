#ifndef ETL_COMMON_TYPES_H_
#define ETL_COMMON_TYPES_H_

#include "etl/common/type_traits.h"

namespace etl {
namespace common {

/*
 * A type large enough to hold the maximum size of an object in memory.
 * The standard defines sizeof in relation to this type, so we exploit this.
 */
typedef decltype(sizeof(char(0))) Size;


/*
 * A utility template for finding a type of a given width.
 */
template <Size N, typename... Types> struct SelectSize;

template <Size N, typename Head, typename... Rest>
struct SelectSize<N, Head, Rest...>
  : public Select<sizeof(Head) == N,
                  TypeConstant<Head>,
                  SelectSize<N, Rest...>>::type {};

#define ETL_COMMON_SIZED_INT(n, name) \
  typedef typename SelectSize<n, char, short, int, long, long long>::type name

ETL_COMMON_SIZED_INT(1, Int8);
ETL_COMMON_SIZED_INT(2, Int16);
ETL_COMMON_SIZED_INT(4, Int32);
ETL_COMMON_SIZED_INT(8, Int64);

#undef ETL_COMMON_SIZED_INT

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_TYPES_H_
