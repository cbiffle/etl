#ifndef _ETL_MATH_AFFINE_H_INCLUDED
#define _ETL_MATH_AFFINE_H_INCLUDED

#include <cmath>

#include "etl/algorithm.h"
#include "etl/math/matrix.h"
#include "etl/math/vector.h"

namespace etl {
namespace math {

/*******************************************************************************
 * IMPLEMENTATION DETAIL - NOT PUBLIC API
 *
 * Transformation helpers.
 */

namespace _affine {
  template <
    std::size_t d,
    typename T,
    Orient o,
    std::size_t... N
  >
  constexpr auto augment_helper(Vector<d, T, o> const & v,
                                IndexSequence<N...>)
      -> Vector<d + 1, T, o> {
    return { get<N>(v)..., T{1} };
  }

  template <
    std::size_t d,
    typename T,
    Orient o,
    std::size_t... N
  >
  constexpr auto project_helper(Vector<d, T, o> const & v,
                                IndexSequence<N...>)
      -> Vector<d - 1, T, o> {
    return get<N...>(v) / get<d - 1>(v);
  }

  template <std::size_t row, std::size_t n, typename T, std::size_t... N>
  constexpr auto translate_row(Vector<n, T> disp, IndexSequence<N...> ns)
      -> Vector<n + 1, T, Orient::row> {
    return { (row == N ? T{1}
                       : N == n ? get<min(row, n - 1)>(disp) : T{0})... };
  }

  template <std::size_t n, typename T, std::size_t... N>
  constexpr auto translate_(Vector<n, T> disp, IndexSequence<N...> ns)
      -> Matrix<n + 1, n + 1, T> {
    return { translate_row<N>(disp, ns)... };
  }

  template <std::size_t row, std::size_t n, typename T, std::size_t... N>
  constexpr auto scale_row(Vector<n, T> scale, IndexSequence<N...> ns)
      -> Vector<n + 1, T, Orient::row> {
    // N is the column index here.
    return { (row != N ? T{0}
                       : row < n ? get<min(row, n-1)>(scale)
                                 : T{1}) ... };
  }

  template <std::size_t n, typename T, std::size_t... N>
  constexpr auto scale_(Vector<n, T> scale, IndexSequence<N...> ns)
      -> Matrix<n + 1, n + 1, T> {
    return { scale_row<N>(scale, ns)... };
  }
}  // namespace _affine

/*******************************************************************************
 * Transformation-related utilities.
 */

/*
 * Augments a point in Euclidean space with an extra 1, placing it on the
 * w=1 plane in homogeneous space.
 *
 * In other words, converts a coordinate into a form that can be used with the
 * augmented matrices produced by the transformations below.
 */
template <
  std::size_t d,
  typename T,
  Orient o
>
constexpr Vector<d + 1, T, o> augment(Vector<d, T, o> const & v) {
  return _affine::augment_helper(v, MakeIndexSequence<d>{});
}

/*
 * Projects a point in homogeneous space back into Euclidean space, using a
 * perspective divide to move it onto the w=1 plane and then dropping that
 * coordinate.
 *
 * In other words, converts a coordinate back from the form produced by
 * `augment(v)` above.
 */
template <
  std::size_t d,
  typename T,
  Orient o
>
constexpr Vector<d - 1, T, o> project(Vector<d, T, o> const & v) {
  return _affine::project_helper(v, MakeIndexSequence<d - 1>{});
}


/*******************************************************************************
 * General transformations.
 */

/*
 * Given displacements in n dimensions, produces an augmented translation
 * matrix (n+1 x n+1 in size).
 */
template <std::size_t n, typename T>
constexpr auto translate(Vector<n, T> disp)
    -> Matrix<n + 1, n + 1, T> {
  return _affine::translate_(disp, MakeIndexSequence<n + 1>{});
}

/*
 * Given factors in n dimensions, produces an augmented scale matrix (n+1 x n+1
 * in size).
 */
template <std::size_t n, typename T>
constexpr auto scale(Vector<n, T> scale)
    -> Matrix<n + 1, n + 1, T> {
  return _affine::scale_(scale, MakeIndexSequence<n + 1>{});
}


/*******************************************************************************
 * Transformations that I haven't bothered to generalize yet.
 */

template <typename T>
constexpr Mat4<T> rotate_z(T a) {
  using namespace std;
  return {
    {cos(a), -sin(a), 0, 0},
    {sin(a),  cos(a), 0, 0},
    {0,       0,      1, 0},
    {0,       0,      0, 1},
  };
}

template <typename T>
constexpr Mat4<T> rotate_y(T a) {
  using namespace std;
  return { 
    {cos(a),  0, sin(a), 0},
    {0,       1, 0,      0},
    {-sin(a), 0, cos(a), 0},
    {0,       0, 0,      1},
  };
}

template <typename T>
constexpr Mat4<T> rotate_x(T a) {
  return {
    {1, 0,      0,       0},
    {0, cos(a), -sin(a), 0},
    {0, sin(a), cos(a),  0},
    {0, 0,      0,       1},
  };
}

template <typename T>
constexpr Mat4<T> ortho(T left, T top,
                        T right, T bottom,
                        T near, T far) {
  return {
    {2/(right - left), 0, 0, -(right + left)/(right - left)},
    {0, 2/(top - bottom), 0, -(top + bottom)/(top - bottom)},
    {0, 0, -2/(far - near), -(far + near)/(far - near)},
    {0, 0, 0, 1},
  };
}

template <typename T>
constexpr Mat4<T> persp(T l, T t,
                        T r, T b,
                        T n, T f) {
  return {
    {2*n/(r-l), 0, (r+l)/(r-l), 0},
    {0, 2*n/(t-b), (t+b)/(t-b), 0},
    {0, 0, -(f+n)/(f-n), -2*f*n/(f-n)},
    {0, 0, -1, 0},
  };
}

template <typename T>
constexpr Mat3<T> rotate(T a) {
  using namespace std;
  return {
    { cos(a), -sin(a), 0 },
    { sin(a), cos(a),  0 },
    { 0,      0,       1 },
  };
}


}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_AFFINE_H_INCLUDED
