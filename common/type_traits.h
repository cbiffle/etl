#ifndef ETL_COMMON_TYPE_TRAITS_H_
#define ETL_COMMON_TYPE_TRAITS_H_

#include "etl/common/attribute_macros.h"

namespace etl {
namespace common {

/*
 * RemoveReference<T>::type strips any reference type from T.
 */

template <typename T>
struct RemoveReference {
  typedef T type;
};

template <typename T>
struct RemoveReference<T &> {
  typedef T type;
};

template <typename T>
struct RemoveReference<T &&> {
  typedef T type;
};


/*
 * Promoting values to types.
 */

template <typename T, T V>
struct IntegralConstant {
  static constexpr T value = V;
  typedef T value_type;
  typedef IntegralConstant<T, V> type;
  constexpr operator T() { return V; }
};

typedef IntegralConstant<bool, true>  TrueType;
typedef IntegralConstant<bool, false> FalseType;

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_TYPE_TRAITS_H_
