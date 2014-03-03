#ifndef ETL_COMMON_ALGORITHM_H_
#define ETL_COMMON_ALGORITHM_H_

namespace etl {
namespace common {

template <typename T>
T const min(T const &a, T const &b) {
  return a < b ? a : b;
}

template <typename T>
T const max(T const &a, T const &b) {
  return a > b ? a : b;
}

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_ALGORITHM_H_
