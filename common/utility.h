#ifndef ETL_COMMON_UTILITY_H_
#define ETL_COMMON_UTILITY_H_

#include "etl/common/attribute_macros.h"
#include "etl/common/type_traits.h"

namespace etl {
namespace common {

template <typename T>
ETL_INLINE T && forward(typename RemoveReference<T>::type & value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE T && forward(typename RemoveReference<T>::type && value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE typename RemoveReference<T>::type && move(T && value) {
  return static_cast<typename RemoveReference<T>::type &&>(value);
}

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_UTILITY_H_
