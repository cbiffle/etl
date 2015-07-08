#ifndef _ETL_MATH_AFFINE_TRANSFORM_H_INCLUDED
#define _ETL_MATH_AFFINE_TRANSFORM_H_INCLUDED

#include <cmath>

#include "etl/algorithm.h"
#include "etl/math/linear_transform.h"
#include "etl/math/matrix.h"
#include "etl/math/vector.h"

namespace etl {
namespace math {
namespace affine_transform {

/*******************************************************************************
 * IMPLEMENTATION DETAIL - NOT PUBLIC API
 *
 * Transformation helpers.
 */

namespace _priv {
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
    std::size_t r,
    std::size_t n,
    typename T,
    std::size_t... N
  >
  constexpr auto augment_row_helper(Matrix<n, n, T> const & m,
                                    IndexSequence<N...>)
      -> MatrixRow<n + 1, T> {
    return {get<N>(m.row[r])..., 0};
  }

  template <
    std::size_t n,
    typename T,
    std::size_t... N
  >
  constexpr auto augment_helper(Matrix<n, n, T> const & m,
                                IndexSequence<N...> ns)
      -> Matrix<n + 1, n + 1, T> {
    return {
      augment_row_helper<N>(m, ns)...,
      {((void) N, T{0})..., T{1}},
    };
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
}  // namespace _priv


/*******************************************************************************
 * Augmentations and projections for moving between affine and linear
 * transforms (euclidean and homogeneous coordinate spaces).
 */

/*
 * Augments a point in Euclidean space with an extra 1, placing it on the
 * w=1 plane in homogeneous space.
 *
 * In other words, converts a basic vector into a form that can be used with
 * affine transforms.
 *
 * To convert back, use `project`.
 */
template <
  std::size_t d,
  typename T,
  Orient o
>
constexpr Vector<d + 1, T, o> augment(Vector<d, T, o> const & v) {
  return _priv::augment_helper(v, MakeIndexSequence<d>{});
}

/*
 * Augments an NxN matrix with another row/column taken from the identity
 * matrix.  That is, it's mostly zeroes except for the lower-right corner.
 */
template <
  std::size_t n,
  typename T
>
constexpr Matrix<n + 1, n + 1, T> augment(Matrix<n, n, T> const & m) {
  return _priv::augment_helper(m, MakeIndexSequence<n>{});
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
  return _priv::project_helper(v, MakeIndexSequence<d - 1>{});
}


/*******************************************************************************
 * General transformations.
 */

/*
 * Given displacements in N dimensions, produces an N+1xN+1 augmented
 * translation matrix.
 */
template <std::size_t n, typename T>
constexpr auto translate(Vector<n, T> disp) -> Matrix<n + 1, n + 1, T> {
  return _priv::translate_(disp, MakeIndexSequence<n + 1>{});
}

/*
 * Given factors in N dimensions, produces an N+1xN+1 augmented scale matrix.
 */
template <std::size_t n, typename T>
constexpr auto scale(Vector<n, T> scale) -> Matrix<n + 1, n + 1, T> {
  return augment(linear_transform::scale(scale));
}


/*******************************************************************************
 * Transformations that I haven't bothered to generalize yet.
 */

/*
 * A matrix that rotates by `a` radians around the Z axis.
 */
template <typename T>
constexpr Mat4<T> rotate_z(T a) {
  return augment(linear_transform::rotate_z(a));
}

/*
 * A matrix that rotates by `a` radians around the Y axis.
 */
template <typename T>
constexpr Mat4<T> rotate_y(T a) {
  return augment(linear_transform::rotate_y(a));
}

/*
 * A matrix that rotates by `a` radians around the X axis.
 */
template <typename T>
constexpr Mat4<T> rotate_x(T a) {
  return augment(linear_transform::rotate_x(a));
}

/*
 * A matrix that rotates by `a` radians around the origin of a two-dimensional
 * space.
 */
template <typename T>
constexpr Mat3<T> rotate(T a) {
  return augment(linear_transform::rotate(a));
}

/*
 * Orthographic projection of the given view volume.
 */
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

/*
 * Perpsective projection of the given view frustum.  The left/right and
 * top/bottom coordinates are of the near clip plane; this, combined with
 * the distance to the near clip plane, determines the field of view.
 */
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

}  // namespace affine_transform
}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_AFFINE_TRANSFORM_H_INCLUDED
