#ifndef _ETL_MATH_VECTOR_H_INCLUDED
#define _ETL_MATH_VECTOR_H_INCLUDED

/*
 * Vectors, in the mathematical sense, not the C++ STL sense.
 *
 * Wikipedia's too-thorough-to-be-valuable discussion:
 *   https://en.wikipedia.org/wiki/Vector_(mathematics_and_physics)
 *
 * For our purposes, a vector is a fixed-length collection of scalars, all of
 * which have the same type.
 *
 * Different types of vectors are represented by instantiations of the `Vector`
 * template, e.g. `Vector<3, float>`.  Template aliases are provided as
 * shorthand for common types, e.g. `Vec3<float>` and `Vec3f`.
 *
 *
 * Column and Row Vectors
 * ======================
 *
 * Vectors come in two flavors, which affect how they interact with each other
 * and with matrices (see matrix.h): column vectors and row vectors.
 *
 * Column vectors are the default in this library.  See:
 *   http://chrishecker.com/Column_vs_row_vectors
 *
 * Column and row vectors can't be mixed, but you can convert one to the other
 * using `transpose(v)`.
 * 
 *
 * Operations on Vectors
 * =====================
 *
 * This discussion refers to some vector type `V`.  I'll get more specific only
 * when required.
 *
 * Basically all vector operations that don't require mutation are declared as
 * constexpr optimistically.  Fully constexpr vector support relies on
 * constexpr versions of `sqrt`, among other things, that are not required by
 * the C++ standard as of C++11.  However, GCC provides these as an extension,
 * so if you're using GCC with `std=gnu++11` or later, you should have constexpr
 * vectors.  (Note that Clang 3.6.1 does not agree.)
 * 
 * Static attributes
 * -----------------
 *
 * `V::dim` is an integer constant giving the number of dimensions (elements)
 * in the vector type.
 *
 * `V::orient` gives the vector type's orientation, either `Orient::col` or
 * `Orient::row`.
 *
 * `V::Transposed` is a vector type of the same size and type as `V` but with
 * opposed orientation.
 *
 * `V::Element` is the type of the vector's scalar elements.
 *
 * `V::WithType<S>` is a vector type of the same size and orientation as `V`
 * but with scalar element type `S`.
 *
 * Creation
 * --------
 *
 * You can create vectors four ways:
 * - Default: `V{}`, which default-constructs the elements.  This is implicit
 *   and can often be written `{}`.
 * - Elements: `V{a, b, c...}`.  This is also implicit and can often be written
 *   `{a, b, c...}`.
 * - Repeat: `V{scalar}`, which repeats a single scalar into all positions
 *   (explicit).
 * - Copy/conversion: `V{vector}`, which is treated as an explicit conversion
 *   and will cast elements into a new type and/or alter the orientation (see
 *   below).
 *
 * The `transpose(v)` function can also be used to derive a row vector from a
 * column vector, or vice versa.  Row and column vectors cannot be mixed in any
 * of the operations below.
 *
 * Arithmetic
 * ----------
 *
 * Once you have some vectors, you can combine them element-wise using the
 * standard C arithmetic operators:
 * - `-v1` inverts the vector (element-wise negation).
 * - `v1 + v2` is the vector sum (element-wise sum).
 * - `v1 - v2` is vector subtraction.
 * - `v1 * v2` is element-wise multiplication (*not* dot or vector product).
 * - `v1 / v2` is element-wise division.
 *
 * All those operations are also available in compound assignment form (e.g.
 * `v1 += v2`).
 *
 * `v1 == v2` and `v1 != v2` perform element-wise comparison and then reduce
 * the result to a single `bool`.
 *
 * Vector-specific product types are provided as functions:
 * - `dot(v1, v2)` is the dot product.
 * - `cross(v1, v2)` is the cross product (only defined for 3-vectors).
 * - `norm(v)` is the vector norm (length or magnitude) and `norm_squared(v)`
 *   is the square of the norm (cheaper to compute and sometimes useful).
 * - `normalized(v)` is a vector pointing in the same direction as `v` but with
 *   unit length.
 * - `transpose(v)` is a vector with the same elements as `v` but opposed
 *   orientation.
 *
 * This library doesn't currently provide for direct operations between vectors
 * and scalars, though the element repetition syntax makes promoting a scalar
 * to a vector relatively compact: `v1 * V{4}`.
 *
 * Element access and shuffling
 * ----------------------------
 *
 * To access the elements of the vector, there are two options, both checked
 * at compile time:
 * - `get<X>(v)` accesses element `X` (0-based) of vector `v`.  This is the
 *   most general method.  It can also be written using member notation:
 *   `v.get<X>()`.
 * - For vectors of 2, 3, or 4 elements, the elements can be accessed using
 *   field notation by their traditional names: `v.x`, `v.y`, `v.z`, `v.w`.
 *
 * To derive a new vector by shuffling, the `get` template can accept more than
 * one index: `v.get<0, 1, 0>()` produces a 3-vector of the same type as `v`
 * whose elements are equal to element 0, 1, and 0 again of `v`.
 *
 * Combinators
 * -----------
 *
 * To simplify lifting an operation over scalars to an operation over vectors,
 * this library provides two kinds of combinators, `parallel` and `horizontal`.
 *
 * A `parallel` combinator takes an operation over scalars and converts it
 * into an element-wise operation over vectors.  `parallel` is available in
 * both unary and binary forms, e.g.
 *
 *    // Add corresponding vector elements (equivalent to operator+)
 *    auto sum = parallel(v1, v2, [](float a, float b) { return a + b; });
 *    // Square each element.
 *    auto sqr = parallel(v, [](float a) { return a * a; });
 *
 * A `horizontal` combinator takes an operation of scalars and applies it
 * *within* a single vector to that vector's elements, reducing them to a scalar
 * result.  For example, to sum the elements of a vector:
 *
 *    auto sum = horizontal(v, [](float a, float b) { return a + b; });
 *
 * Unlike combinators in more civilized languages, these do not return a
 * functor, but apply themselves immediately, for efficiency reasons.
 *
 * Both `parallel` and `horizontal` are implemented `constexpr` and can be
 * used at compile time if the scalar operation is also `constexpr` (which
 * unfortunately excludes all the examples above due to the use of lambdas).
 */

#include "etl/utility.h"
#include "etl/integer_sequence.h"
#include "etl/type_list.h"
#include "etl/functor.h"

namespace etl {
namespace math {

/*
 * Vectors come in two flavors, column (the typical vector) and row.  We refer
 * to these as "orientations" of the vector.  The vector's orientation affects
 * the role it can play in certain operators.
 */
enum class Orient : bool { row, col };

/*
 * `flip(o)` produces the opposed orientation to `o`.
 */
constexpr inline Orient flip(Orient o) {
  return o == Orient::row ? Orient::col : Orient::row;
}

/*
 * Base type for vectors so they can be recognized at the type level.
 */
struct VectorTag {};

/*
 * Predicate template for checking whether something is a vector.
 */
template <typename T>
using IsVector = std::is_base_of<VectorTag, T>;

/*
 * Forward declaration of the final Vector template.
 */
template <std::size_t _dim, typename T, Orient _orient = Orient::col>
struct Vector;


/*******************************************************************************
 * VectorBase provides the actual member variables for the vector's components.
 * It is specialized to common sizes so that the variables can have the usual
 * names (x, y, z, so forth).
 *
 * Its specializations contain only code for operating on the vector's
 * components.  To avoid repeating ourselves, actual operations like addition
 * or dot product should be defined on the general Vector template, below.
 *
 * VectorBase is parameterized with a TypeList containing its components.
 * This is an odd use of a TypeList, since all its types are equal (they're all
 * T), but is the only way to generate the element-wise constructor sequence
 * we want.
 */

namespace _vec {  // implementation details

/*
 * Common prototype.
 */
template <std::size_t dim, typename T, Orient _orient, typename TL>
struct VectorBase;

/*
 * The catch-all specialization for vectors of 0, 1, or many components.
 *
 * Such vectors do not have named fields; the fields are accessed as
 * elements[n].
 */
template <std::size_t dim, typename T, Orient _orient, typename... Es>
struct VectorBase<dim, T, _orient, TypeList<Es...>> : public VectorTag {
  T elements[dim];

  // Default ctor.
  constexpr VectorBase() = default;

  // Element-wise ctor
  constexpr VectorBase(Es const & ...args) : elements{args...} {}

  // Repetition ctor
  constexpr explicit VectorBase(T val)
      : VectorBase{repl(val, MakeIndexSequence<dim>{})} {}

  // Transpose ctor
  constexpr explicit VectorBase(
      VectorBase<dim, T, flip(_orient), TypeList<Es...>> const & other)
    : elements{other.elements} {}

  template <std::size_t n>
  constexpr auto get() const
      -> typename std::enable_if<n < dim, T>::type {
    return elements[n];
  }

  template <std::size_t n>
  auto get()
      -> typename std::enable_if<n < dim, T &>::type {
    return elements[n];
  }

private:
  template <std::size_t... D>
  static constexpr VectorBase repl(T val, IndexSequence<D...>) {
    return { ((void) D, val)... };
  }
};


/*
 * Specialization of VectorBase for 2-vectors with fields named x, y.
 */
template <typename T, Orient _orient, typename... Es>
struct VectorBase<2, T, _orient, TypeList<Es...>> : public VectorTag {
  T x, y;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_) : x{x_}, y{y_} {}
  // Repetition ctor
  constexpr explicit VectorBase(T v) : x{v}, y{v} {}

  // Transpose ctor
  constexpr explicit VectorBase(
      VectorBase<2, T, flip(_orient), TypeList<Es...>> const & other)
    : x{other.x}, y{other.y} {}

  template <std::size_t n>
  constexpr auto get() const
      -> typename std::enable_if<n < 2, T>::type {
    return n == 0 ? x : y;
  }

  template <std::size_t n>
  auto get()
      -> typename std::enable_if<n < 2, T &>::type {
    return n == 0 ? x : y;
  }
};

/*
 * Specialization of VectorBase for 3-vectors with fields named x, y, z.
 */
template <typename T, Orient _orient, typename... Es>
struct VectorBase<3, T, _orient, TypeList<Es...>> : public VectorTag {
  T x, y, z;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_, T z_) : x{x_}, y{y_}, z{z_} {}
  // Repetition ctor
  constexpr explicit VectorBase(T v) : x{v}, y{v}, z{v} {}

  // Transpose ctor
  constexpr explicit VectorBase(
      VectorBase<3, T, flip(_orient), TypeList<Es...>> const & other)
    : x{other.x}, y{other.y}, z{other.z} {}

  template <std::size_t n>
  constexpr auto get() const
      -> typename std::enable_if<n < 3, T>::type {
    return n == 0 ? x
                  : n == 1 ? y
                           : z;
  }

  template <std::size_t n>
  auto get()
      -> typename std::enable_if<n < 3, T &>::type {
    return n == 0 ? x
                  : n == 1 ? y
                           : z;
  }
};

/*
 * Specialization of VectorBase for 4-vectors with fields named x, y, z, w.
 */
template <typename T, Orient _orient, typename... Es>
struct VectorBase<4, T, _orient, TypeList<Es...>> : public VectorTag {
  T x, y, z, w;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_, T z_, T w_)
    : x{x_}, y{y_}, z{z_}, w{w_} {}
  // Repetition ctor
  constexpr explicit VectorBase(T v) : x{v}, y{v}, z{v}, w{v} {}

  // Transpose ctor
  constexpr explicit VectorBase(
      VectorBase<4, T, flip(_orient), TypeList<Es...>> const & other)
    : x{other.x}, y{other.y}, z{other.z}, w{other.w} {}

  template <std::size_t n>
  constexpr auto get() const
      -> typename std::enable_if<n < 4, T>::type {
    return n == 0 ? x
                  : n == 1 ? y
                           : n == 2 ? z
                                    : w;
  }

  template <std::size_t n>
  auto get()
      -> typename std::enable_if<n < 3, T &>::type {
    return n == 0 ? x
                  : n == 1 ? y
                           : n == 2 ? z
                                    : w;
  }
};

template <std::size_t dim, typename T, Orient orient>
using VectorBaseHelper = VectorBase<dim, T, orient, Repeat<T, dim>>;

}  // namespace _vec

/*******************************************************************************
 * Vector is the actual vector type, derived from VectorBase.
 *
 * Most C++ operators are implemented as component-wise operations, meaning that
 * '*' is not a vector product, but rather a vector of scalar products.  Vector
 * specific operations are free functions defined below.
 */

template <std::size_t _dim, typename T, Orient _orient>
struct Vector : public _vec::VectorBaseHelper<_dim, T, _orient> {
  static constexpr std::size_t dim = _dim;
  static constexpr Orient orient = _orient;

  using Transposed = Vector<_dim, T, flip(_orient)>;
  using Element = T;
  using Base = _vec::VectorBaseHelper<_dim, T, _orient>;

  constexpr Vector() = default;
  constexpr Vector(Vector const &) = default;

  using Base::Base;

  template <typename S>
  constexpr explicit Vector(Vector<_dim, S, _orient> const & other)
    : Vector(parallel(other, functor::Construct<S, T>{})) {}

  template <typename S>
  using WithType = Vector<_dim, S, _orient>;

  template <std::size_t I>
  constexpr T get() const {
    return this->Base::template get<I>();
  }

  template <std::size_t I0, std::size_t I1, std::size_t... I>
  constexpr auto get() const
      -> Vector<sizeof...(I) + 2, T, _orient> {
    return Vector<sizeof...(I) + 2, T, _orient>{
      this->template get<I0>(),
      this->template get<I1>(),
      this->template get<I>()...
    };
  }
};

template <typename T, Orient orient = Orient::col>
using Vec2 = Vector<2, T, orient>;

template <typename T, Orient orient = Orient::col>
using Vec3 = Vector<3, T, orient>;

template <typename T, Orient orient = Orient::col>
using Vec4 = Vector<4, T, orient>;

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;

using Vec2i = Vec2<int>;
using Vec3i = Vec3<int>;
using Vec4i = Vec4<int>;

template <std::size_t I, std::size_t dim, typename T, Orient orient>
constexpr auto get(Vector<dim, T, orient> const & v)
    -> typename std::enable_if<I < dim, T>::type {
  return v.template get<I>();
}

template <std::size_t I0, std::size_t I1, std::size_t... I,
          std::size_t dim, typename T, Orient orient>
constexpr auto get(Vector<dim, T, orient> const & v)
      -> Vector<sizeof...(I) + 2, T, orient> {
  return v.template get<I0, I1, I...>();
}

/*******************************************************************************
 * Parallel and horizontal combinators.
 *
 * These templates lift operations on scalars to operations on vectors in one
 * of two ways:
 * - Parallel combinators apply a scalar operation separately to each element
 *   of a vector, or to corresponding elements of two vectors.
 * - Horizontal combinators apply a scalar operation across the elements of
 *   a vector, producing a scalar.
 */

namespace _vec {
  template <
    std::size_t dim,
    typename T,
    Orient orient,
    typename F,
    std::size_t... I
  >
  constexpr auto parallel_(Vector<dim, T, orient> const & v,
                           F && fn,
                           IndexSequence<I...>)
      -> Vector<dim, decltype(fn(T{})), orient> {
    return { fn(get<I>(v))... };
  }

  template <
    std::size_t dim,
    typename T,
    typename S,
    Orient orient,
    typename F,
    std::size_t... I
  >
  constexpr auto parallel_(Vector<dim, T, orient> const & a,
                           Vector<dim, S, orient> const & b,
                           F && fn,
                           IndexSequence<I...>)
      -> Vector<dim, decltype(fn(T{}, S{})), orient> {
    return { fn(get<I>(a), get<I>(b))... };
  }
  
  template <
    std::size_t dim,
    typename T,
    Orient orient,
    typename F,
    std::size_t I0,
    std::size_t I1,
    std::size_t... I
  >
  constexpr auto horizontal_(Vector<dim, T, orient> const & v,
                             F && fn,
                             IndexSequence<I0, I1, I...>)
      -> T {
    return fn(get<I0>(v),
              horizontal_(v, forward<F>(fn), IndexSequence<I1, I...>{}));
  }
  
  template <
    std::size_t dim,
    typename T,
    Orient orient,
    typename F,
    std::size_t I0
  >
  constexpr auto horizontal_(Vector<dim, T, orient> const & v,
                             F && fn,
                             IndexSequence<I0>)
      -> T {
    return get<I0>(v);
  }

}  // namespace _vec

/*
 * Produces a new vector by applying 'fn' to each element of 'v'.
 */
template <
  std::size_t dim,
  typename T,
  Orient orient,
  typename F
>
constexpr auto parallel(Vector<dim, T, orient> const & v, F && fn)
    -> decltype(parallel_(v, forward<F>(fn), MakeIndexSequence<dim>{})) {
  return parallel_(v, forward<F>(fn), MakeIndexSequence<dim>{});
}

/*
 * Produces a new vector by applying 'fn' to corresponding elements of 'a' and
 * 'b'.
 */
template <
  std::size_t dim,
  typename T,
  typename S,
  Orient orient,
  typename F,
  std::size_t... I
>
constexpr auto parallel(Vector<dim, T, orient> const & a,
                        Vector<dim, S, orient> const & b,
                        F && fn)
    -> decltype(parallel_(a, b, forward<F>(fn), MakeIndexSequence<dim>{})) {
  return parallel_(a, b, forward<F>(fn), MakeIndexSequence<dim>{});
}

/*
 * Applies 'fn' to pairs of elements in 'v' until all elements have been
 * considered, and returns the result.
 */
template <
  std::size_t dim,
  typename T,
  Orient orient,
  typename F,
  std::size_t... I
>
constexpr auto horizontal(Vector<dim, T, orient> const & v,
                          F && fn)
    -> decltype(horizontal_(v, forward<F>(fn), MakeIndexSequence<dim>{})) {
  return horizontal_(v, forward<F>(fn), MakeIndexSequence<dim>{});
}


/*******************************************************************************
 * Lifted arithmetic operators.  These implementations follow a common pattern
 * using the parallel combinator.
 */

// Negation
template <std::size_t dim, typename T, Orient orient>
constexpr auto operator-(Vector<dim, T, orient> const & v)
    -> Vector<dim, decltype(-T{}), orient> {
  return parallel(v, functor::Negate<T>{});
}

// Addition
template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto operator+(Vector<dim, T, orient> const & a,
                         Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} + S{}), orient> {
  return parallel(a, b, functor::Add<T, S>{});
}

// Subtraction
template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto operator-(Vector<dim, T, orient> const & a,
                         Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} - S{}), orient> {
  return parallel(a, b, functor::Subtract<T, S>{});
}

// Multiplication
template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto operator*(Vector<dim, T, orient> const & a,
                         Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} * S{}), orient> {
  return parallel(a, b, functor::Multiply<T, S>{});
}

// Division
template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto operator/(Vector<dim, T, orient> const & a,
                         Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} / S{}), orient> {
  return parallel(a, b, functor::Divide<T, S>{});
}

/*******************************************************************************
 * Compound assignment.  These are derived from the binary operators using a
 * common pattern.
 */

template <std::size_t dim, typename T, typename S, Orient orient>
Vector<dim, T, orient> & operator+=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target + other;
}

template <std::size_t dim, typename T, typename S, Orient orient>
Vector<dim, T, orient> & operator-=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target - other;
}

template <std::size_t dim, typename T, typename S, Orient orient>
Vector<dim, T, orient> & operator*=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target * other;
}

template <std::size_t dim, typename T, typename S, Orient orient>
Vector<dim, T, orient> & operator/=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target / other;
}

/*******************************************************************************
 * Comparison operators.  These combine element-wise comparison with a
 * horizontal conjunction.
 */

template <std::size_t dim, typename T, typename S, Orient orient>
constexpr bool operator==(Vector<dim, T, orient> const & a,
                          Vector<dim, S, orient> const & b) {
  return horizontal(parallel(a, b, functor::Equal<T, S>{}),
                    functor::LogicalAnd<bool, bool>{});
}

template <std::size_t dim, typename T, typename S, Orient orient>
constexpr bool operator!=(Vector<dim, T, orient> const & a,
                          Vector<dim, S, orient> const & b) {
  return horizontal(parallel(a, b, functor::NotEqual<T, S>{}),
                    functor::LogicalOr<bool, bool>{});
}


/*******************************************************************************
 * Vector-specific operations.
 */

template <std::size_t dim, typename T, Orient orient>
constexpr auto transpose(Vector<dim, T, orient> const & v)
    -> Vector<dim, T, flip(orient)> {
  return Vector<dim, T, flip(orient)>{v};
}

template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto dot(Vector<dim, T, orient> const & a,
                   Vector<dim, S, orient> const & b)
    -> decltype(T{} * S{}) {
  return horizontal(a * b, functor::Add<T, S>{});
}

template <std::size_t dim, typename T, Orient orient>
constexpr auto norm_squared(Vector<dim, T, orient> const & a)
    -> decltype(dot(a, a)) {
  return dot(a, a);
}

template <std::size_t dim, typename T, Orient orient>
constexpr auto norm(Vector<dim, T, orient> const & a)
    -> decltype(norm_squared(a)) {
  using namespace std;
  return sqrt(norm_squared(a));
}

template <std::size_t dim, typename T, Orient orient>
inline constexpr auto normalized(Vector<dim, T, orient> const & a)
    -> Vector<dim, T, orient> {
  return a / Vector<dim, T, orient>{norm(a)};
}

template <typename T, typename S, Orient orient>
inline constexpr auto cross(Vec3<T, orient> const & a,
                            Vec3<S, orient> const & b)
    -> Vec3<decltype(T{} * S{} - T{} * S{}), orient> {
  return (get<1, 2, 0>(a) * get<2, 0, 1>(b))
       - (get<2, 0, 1>(a) * get<1, 2, 0>(b));
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_VECTOR_H_INCLUDED
