#ifndef ETL_UTILITY_H_
#define ETL_UTILITY_H_

#include "etl/attribute_macros.h"
#include "etl/type_traits.h"

namespace etl {

template <typename T>
ETL_INLINE T && forward(typename RemoveReference<T>::Type & value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE T && forward(typename RemoveReference<T>::Type && value) {
  return static_cast<T &&>(value);
}

template <typename T>
ETL_INLINE typename RemoveReference<T>::Type && move(T && value) {
  return static_cast<typename RemoveReference<T>::Type &&>(value);
}

}  // namespace etl

#endif  // ETL_UTILITY_H_
