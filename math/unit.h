#ifndef _ETL_MATH_UNIT_H_INCLUDED
#define _ETL_MATH_UNIT_H_INCLUDED

/*
 * Marks a type as having unit length (magnitude, etc.).
 *
 * Abstract Explanation
 * ====================
 *
 * For some type `T` that has a notion of magnitude (written `mag(t)`), a
 * `Unit<T>` describes the subset of values where we can prove the property
 * `mag(t) == 1` at compile time.
 *
 *
 * Concrete Explanation
 * ====================
 *
 * Consider `Vec2f`, a vector made up of two floats.  The magnitude of such
 * a vector `mag(v)` can be computed as 
 *
 *     sqrt(v.x * v.x + v.y * v.y)
 *
 * We can *normalize* the vector (written `normalized(v)`) by dividing each
 * of its components by `mag(v)`:
 *
 *    {v.x / mag(v), v.y / mag(v)}
 *
 * The result of this is called a *unit vector*.
 *
 * It's not unusual for operations to require unit vectors as input.  For
 * example, the dot product `dot(u, v)` produces the cosine of the angle
 * between the vectors `u` and `v` -- but only if `u` and `v` are unit
 * vectors.
 *
 * We could write our code to silently assume our inputs are unit, perhaps
 * mentioning this in a comment.  This is cheap and general, but error-prone.
 *
 * We could instead compute `dot(normalized(u), normalized(v))`.  This is
 * robust, but expensive if the inputs are already normalized.
 *
 * Only the caller knows whether the values we're receiving are normalized.
 * If only we had some way to require normalized inputs at the type level, or
 * -- better yet -- overload to select different algorithms depending on whether
 * the inputs are normalized.
 *
 * This is what `Unit<Vec2f>` (aliased as `UVec3f` in vector.h) provides.
 * There are a few ways to get a `UVec3f`, but the easiest one is by
 * normalizing some other vector.
 *
 * Of course, there is also a back door: if you're *really sure* that some
 * value is guaranteed to be unit, you can use `UVec3f::from_unchecked(v)` to
 * stamp the unit type on it.
 */

#include <type_traits>

namespace etl {
namespace math {


/*******************************************************************************
 * Recognizing unit types at compile time.
 */

// Forward declaration of Unit template for the predicate template below.
template <typename X>
struct Unit;

/*
 * Helper for IsUnit; predicate template for recognizing a `Unit<T>`.
 */
template <typename X>
struct IsUnitHelper : std::false_type {};

template <typename X>
struct IsUnitHelper<Unit<X>> : std::true_type {};

/*
 * Predicate template for recognizing that some type `T` is actually a type
 * `Unit<X>` for some unknown `X`.
 */
template <typename T>
using IsUnit = IsUnitHelper<typename std::decay<T>::type>;


/*******************************************************************************
 * The Unit template.
 */

template <typename X>
struct Unit : public X {
  /*
   * The underlying type.
   */
  using Base = X;

  /*
   * There's no obviously correct "default" for a unit length quantity.
   */
  Unit() = delete;

  /*
   * Confers unit status upon an arbitrary value without checking.
   * This is essentially a cast.
   *
   * It is assumed that you know what you're doing.
   */
  static constexpr Unit from_unchecked(X const & x) {
    return Unit{x};
  }

private:
  constexpr explicit Unit(X const & x) : X{x} {}
};


/*******************************************************************************
 * Common operations on things that are Unit-y.
 */

/*
 * Strips unit status.
 *
 * This can be used to explicitly invoke a non-unit overload of an operation.
 * 
 * For example, repeated operations on units may, due to numerical imprecision,
 * cause the magnitude to drift away from 1.  This can be corrected:
 *
 *    normalized(as_nonunit(x))
 *
 * Note that this involves a copy, because C++ has no way of designating
 * reference lifetimes.
 */
template <typename X>
constexpr X as_nonunit(Unit<X> const & u) {
  return X{u};
}

/*
 * Normalizes a value, converting it to unit length.
 */
template <typename X>
constexpr auto normalized(X const & v)
  -> typename std::enable_if<!IsUnit<X>::value, Unit<X>>::type {
  return Unit<X>::from_unchecked(v / mag(v));
}

/*
 * Optimized normalization for units.
 */
template <typename X>
constexpr Unit<X> normalized(Unit<X> const & v) {
  return v;
}

/*
 * Lifts a binary operation over `X` to an equivalent operation over `Unit<X>`.
 * Implementation factor for overloads that want to preserve unit status.
 *
 * Note that, if you want the result to be `constexpr`, `F` needs to be a
 * literal type, and function references are not.
 */
template <typename X, typename Y, typename F>
constexpr auto lift_unit(Unit<X> const & x, Unit<Y> const & y, F && fn)
    -> Unit<ResultOf<F(X, Y)>> {
  return Unit<ResultOf<F(X, Y)>>::from_unchecked(
      fn(as_nonunit(x), as_nonunit(y)));
}

/*
 * Lifts a unary operation over `X` to an equivalent operation over `Unit<X>`.
 * Implementation factor for overloads that want to preserve unit status.
 *
 * Note that, if you want the result to be `constexpr`, `F` needs to be a
 * literal type, and function references are not.
 */
template <typename X, typename F>
constexpr auto lift_unit(Unit<X> const & x, F && fn) -> Unit<ResultOf<F(X)>> {
  return Unit<ResultOf<F(X)>>::from_unchecked(fn(as_nonunit(x)));
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_UNIT_H_INCLUDED
