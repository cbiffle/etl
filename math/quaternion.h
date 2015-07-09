#ifndef _ETL_MATH_QUATERNION_H_INCLUDED
#define _ETL_MATH_QUATERNION_H_INCLUDED

#include <cmath>
#include <type_traits>

#include "etl/math/vector.h"
#include "etl/math/matrix.h"

namespace etl {
namespace math {

struct QuaternionTag {};

template <typename T>
using IsQuaternion = std::is_base_of<QuaternionTag, T>;


/*******************************************************************************
 * Quaternion template and operations.
 */

template <typename T>
struct Quaternion : public QuaternionTag {
  using Element = T;

  T scalar;
  Vec3<T> vector;

  constexpr Quaternion() = default;
  constexpr Quaternion(T w, T x, T y, T z) : scalar{w}, vector{x, y, z} {}
  constexpr Quaternion(T w, Vec3<T> v) : scalar{w}, vector{v} {}
};

/*
 * Convenient shorthand for creating a quaternion from scalar+vector
 * representation.
 */
template <typename T>
constexpr Quaternion<T> quat(T w, Vec3<T> v) {
  return {w, v};
}

/*
 * Computes the quaternion conjugate `q*`.
 */
template <typename T>
constexpr Quaternion<T> conjugate(Quaternion<T> const & q) {
  return { q.scalar, -q.vector };
}

/*
 * Computes the quaternion norm, which is (in implementation) equivalent to the
 * Euclidean vector norm for a 4vec with the same elements.
 */
template <typename T>
constexpr auto norm(Quaternion<T> const & q) -> decltype(T{} * T{}) {
  using namespace std;
  return sqrt(q.scalar * q.scalar + dot(q.vector, q.vector));
}

/*
 * `mag(q)` is another way of saying `norm(q)` by analogy to vector.h.
 */
template <typename T>
constexpr auto mag(Quaternion<T> const & q) -> decltype(norm(q)) {
  return norm(q);
}

/*
 * Multiplies a quaternion by another.  This is the general case for non-unit
 * quaternions; quaternions known to be unit at compile time are handled below.
 */
template <typename T, typename S>
constexpr auto operator*(Quaternion<T> const & p, Quaternion<S> const & q)
    -> Quaternion<decltype(T{} * S{})> {
  return {
    p.scalar * q.scalar - dot(p.vector, q.vector),
    p.scalar * q.vector + q.scalar * p.vector + cross(p.vector, q.vector)
  };
}


/*******************************************************************************
 * Unit quaternion subtype.
 *
 * This is almost identical to Quaternion, except that the methods of
 * construction are more tightly controlled.  To force arbitrary values into
 * a "unit" quaternion without runtime checks or normalization, use the
 * static member factory function 'from_parts'.
 *
 * As a subtype, unit quaternions can be used anywhere a quaternion is required,
 * with the cost that the result of any such operation is likely a non-unit
 * quaternion.
 *
 * Operations that preserve the quaternion norm are overloaded specifically for
 * unit quaternions below.
 */

template <typename T>
struct UnitQuaternion : public Quaternion<T> {
  // There's no obvious "default" or "zero" unit quaternion.
  UnitQuaternion() = delete;

  // Unsafe coercion backdoor, for when you really just want to plug in the
  // components.
  static constexpr UnitQuaternion from_parts(T w, Vec3<T> v) {
    return UnitQuaternion{w, v};
  }

  // Unsafe coercion backdoor, for when you really just want to coerce an
  // arbitrary quaternion.
  static constexpr UnitQuaternion from_arbitrary(Quaternion<T> const & q) {
    return UnitQuaternion{q};
  }

  /*
   * Rotates the 3vec `v` by the rotation described by this unit quaternion.
   */
  constexpr auto rotate(Vec3<T> const & v) const -> Vec3<decltype(T{} * T{})> {
    return (*this * quat(T{0}, v) * conjugate(*this)).vector;
  }

private:
  constexpr explicit UnitQuaternion(T w, Vec3<T> v) : Quaternion<T>{w, v} {}
  constexpr explicit UnitQuaternion(Quaternion<T> q) : Quaternion<T>{q} {}
};

template <typename T>
constexpr UnitQuaternion<T> identity_quat() {
  return UnitQuaternion<T>::from_parts(1, Vec3<T>{0});
}

template <typename T>
constexpr UnitQuaternion<T> normalized(Quaternion<T> const & q) {
  return UnitQuaternion<T>::from_parts(
      q.scalar / norm(q),
      q.vector / norm(q));
}

template <typename T>
constexpr UnitQuaternion<T> normalized(UnitQuaternion<T> const & q) {
  return q;
}

namespace _quat {
  template <typename T>
  constexpr UnitQuaternion<T> rotation_vec_nonunit_step2(Quaternion<T> q) {
    return normalized(quat(q.scalar + norm(q), q.vector));
  }

  template <typename T>
  constexpr UnitQuaternion<T> rotation_vec_unit_step2(T m, Vec3<T> cr) {
    return UnitQuaternion<T>::from_parts(
        T{0.5} * m,
        (T{1} / m) * cr);
  }

  template <typename T>
  constexpr Mat4<T> rotation_matrix_helper(T w, T x, T y, T z) {
    return {
      {w*w + x*x - y*y - z*z, 2*x*y - 2*w*z, 2*x*z + 2*w*y, 0},
      {2*x*y + 2*w*z, w*w - x*x + y*y - z*z, 2*y*z - 2*w*x, 0},
      {2*x*z - 2*w*y, 2*y*z + 2*w*x, w*w - x*x - y*y + z*z, 0},
      {0,             0,             0,                     1},
    };
  }

}  // namespace _quat

/*
 * Finds the unit quaternion describing a rotation of `angle` radians around
 * `axis`.
 */
template <typename T>
constexpr UnitQuaternion<T> rotation(UVec3<T> axis, T angle) {
  using namespace std;
  return UnitQuaternion<T>::from_parts(
      cos(angle/2),
      axis * sin(angle/2)
  );
}

/*
 * Finds the unit quaternion describing a rotation of `angle` radians around
 * `axis`.  This implementation *does not* assume that `axis` is unit.
 */
template <typename T>
constexpr UnitQuaternion<T> rotation(Vec3<T> axis, T angle) {
  return rotation(normalized(axis), angle);
}

/*
 * Finds the unit quaternion that rotates `start` to be `end`.
 */
template <typename T>
constexpr UnitQuaternion<T> rotation(UVec3<T> start, UVec3<T> end) {
  using namespace std;
  return _quat::rotation_vec_unit_step2(
      sqrt(T{2} + T{2} * dot(start, end)),
      cross(start, end));
}

/*
 * Finds the unit quaternion that rotates `start` to be parallel to `end`.
 * This implementation *does not* assume unit vectors.
 */
template <typename T>
constexpr UnitQuaternion<T> rotation(Vec3<T> start, Vec3<T> end) {
  return _quat::rotation_vec_nonunit_step2(
      quat(dot(start, end), cross(start, end)));
}

/*
 * Constructs a 4x4 augmented rotation matrix equivalent to a unit quaternion.
 */
template <typename T>
constexpr Mat4<T> rotation_matrix(UnitQuaternion<T> const & u) {
  return _quat::rotation_matrix_helper(
      u.scalar, u.vector.x, u.vector.y, u.vector.z);
}


/*******************************************************************************
 * Overloads of quaternion operations that preserve the quaternion norm.
 */

template <typename T>
constexpr UnitQuaternion<T> conjugate(UnitQuaternion<T> const & q) {
  return UnitQuaternion<T>::from_parts(q.scalar, -q.vector);
}

template <typename T, typename S>
constexpr auto operator*(UnitQuaternion<T> const & p,
                         UnitQuaternion<S> const & q)
    -> UnitQuaternion<decltype(T{} * S{})> {
  using R = UnitQuaternion<decltype(T{} * S{})>;

  return R::from_arbitrary(Quaternion<T>{p} * Quaternion<S>{q});
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_QUATERNION_H_INCLUDED
