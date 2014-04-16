#ifndef ETL_ALGORITHM_H_
#define ETL_ALGORITHM_H_

namespace etl {

template <typename T>
T const min(T const &a, T const &b) {
  return a < b ? a : b;
}

template <typename T>
T const max(T const &a, T const &b) {
  return a > b ? a : b;
}

}  // namespace etl

#endif  // ETL_ALGORITHM_H_
