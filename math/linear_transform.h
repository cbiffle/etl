#ifndef _ETL_MATH_LINEAR_TRANSFORM_H_INCLUDED
#define _ETL_MATH_LINEAR_TRANSFORM_H_INCLUDED

#include <cmath>

#include "etl/algorithm.h"
#include "etl/math/matrix.h"
#include "etl/math/vector.h"

namespace etl {
namespace math {
namespace linear_transform {

/*******************************************************************************
 * IMPLEMENTATION DETAIL - NOT PUBLIC API
 *
 * Transformation helpers.
 */

namespace _priv {
  template <std::size_t row, std::size_t n, typename T, std::size_t... N>
  constexpr auto scale_row(Vector<n, T> scale, IndexSequence<N...> ns)
      -> Vector<n, T, Orient::row> {
    // N is the column index here.
    return { (row != N ? T{0} : get<row>(scale))... };
  }

  template <std::size_t n, typename T, std::size_t... N>
  constexpr auto scale_(Vector<n, T> scale, IndexSequence<N...> ns)
      -> Matrix<n, n, T> {
    return { scale_row<N>(scale, ns)... };
  }
}  // namespace _priv


/*******************************************************************************
 * General transformations that apply to any matrix size.
 */

/*
 * Given factors in N dimensions, produces an NxN scale matrix.
 */
template <std::size_t n, typename T>
constexpr auto scale(Vector<n, T> scale) -> Matrix<n, n, T> {
  return _priv::scale_(scale, MakeIndexSequence<n>{});
}


/*******************************************************************************
 * Transformations that I haven't bothered to generalize yet.
 */

/*
 * A matrix that rotates by `a` radians around the Z axis.
 */
template <typename T>
constexpr Mat3<T> rotate_z(T a) {
  using namespace std;
  return {
    {cos(a), -sin(a), 0},
    {sin(a),  cos(a), 0},
    {0,       0,      1},
  };
}

/*
 * A matrix that rotates by `a` radians around the Y axis.
 */
template <typename T>
constexpr Mat3<T> rotate_y(T a) {
  using namespace std;
  return { 
    {cos(a),  0, sin(a)},
    {0,       1, 0     },
    {-sin(a), 0, cos(a)},
  };
}

/*
 * A matrix that rotates by `a` radians around the X axis.
 */
template <typename T>
constexpr Mat3<T> rotate_x(T a) {
  return {
    {1, 0,      0,     },
    {0, cos(a), -sin(a)},
    {0, sin(a), cos(a) },
  };
}

/*
 * A matrix that rotates by `a` radians around the origin of a two-dimensional
 * space.
 */
template <typename T>
constexpr Mat2<T> rotate(T a) {
  // TODO: ought to be able to share this with rotate_z; shame we can't see
  // augment() from here.
  using namespace std;
  return {
    { cos(a), -sin(a)},
    { sin(a), cos(a) },
  };
}

}  // namespace linear_transform
}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_LINEAR_TRANSFORM_H_INCLUDED
