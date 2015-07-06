#ifndef _ETL_MATH_VECTOR_H_INCLUDED
#define _ETL_MATH_VECTOR_H_INCLUDED

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

constexpr inline Orient flip(Orient o) {
  return o == Orient::row ? Orient::col : Orient::row;
}

/*
 * Base type for vectors so they can be recognized at the type level.
 */
struct VectorTag {};

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
 */

namespace _vec {  // implementation details

/*
 * The catch-all specialization for vectors of 0, 1, or many components.
 *
 * Such vectors do not have named fields; the fields are accessed as
 * elements[n].
 */
template <std::size_t dim, typename T, Orient _orient>
struct VectorBase : public VectorTag {
  T elements[dim];

  // Default ctor.
  constexpr VectorBase() = default;

  // Element-wise ctor
  template <
    typename ... A,
    typename TL = TypeList<A...>,
    typename = typename std::enable_if<None<IsVector, TL>::value>::type,
    typename = typename std::enable_if<sizeof...(A) == dim>::type
  >
  constexpr VectorBase(A && ... args) : elements{forward<A>(args)...} {}

  // Repetition ctor
  constexpr explicit VectorBase(T val)
      : VectorBase{repl(val, MakeIndexSequence<dim>{})} {}

  // Transpose ctor
  constexpr VectorBase(VectorBase<dim, T, flip(_orient)> const & other)
    : elements{other.elements} {}

  template <std::size_t n>
  constexpr T get() const {
    static_assert(n < dim, "vector element out of range");
    return elements[n];
  }

  template <std::size_t n>
  T & get() {
    static_assert(n < dim, "vector element out of range");
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
template <typename T, Orient _orient>
struct VectorBase<2, T, _orient> : public VectorTag {
  T x, y;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_) : x{x_}, y{y_} {}
  // Repetition ctor
  constexpr VectorBase(T v) : x{v}, y{v} {}

  // Transpose ctor
  constexpr VectorBase(VectorBase<2, T, flip(_orient)> const & other)
    : x{other.x}, y{other.y} {}

  template <std::size_t n>
  constexpr T get() const {
    static_assert(n < 2, "vector element out of range");
    return n == 0 ? x : y;
  }

  template <std::size_t n>
  T & get() {
    static_assert(n < 2, "vector element out of range");
    return n == 0 ? x : y;
  }
};

/*
 * Specialization of VectorBase for 3-vectors with fields named x, y, z.
 */
template <typename T, Orient _orient>
struct VectorBase<3, T, _orient> : public VectorTag {
  T x, y, z;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_, T z_) : x{x_}, y{y_}, z{z_} {}
  // Repetition ctor
  constexpr VectorBase(T v) : x{v}, y{v}, z{v} {}

  // Transpose ctor
  constexpr VectorBase(VectorBase<3, T, flip(_orient)> const & other)
    : x{other.x}, y{other.y}, z{other.z} {}

  template <std::size_t n>
  constexpr T get() const {
    static_assert(n < 3, "vector element out of range");
    return n == 0 ? x
                  : n == 1 ? y
                           : z;
  }

  template <std::size_t n>
  T & get() {
    static_assert(n < 3, "vector element out of range");
    return n == 0 ? x
                  : n == 1 ? y
                           : z;
  }
};

/*
 * Specialization of VectorBase for 4-vectors with fields named x, y, z, w.
 */
template <typename T, Orient _orient>
struct VectorBase<4, T, _orient> : public VectorTag {
  T x, y, z, w;

  // Default ctor
  constexpr VectorBase() = default;
  // Element-wise ctor
  constexpr VectorBase(T x_, T y_, T z_, T w_)
    : x{x_}, y{y_}, z{z_}, w{w_} {}
  // Repetition ctor
  constexpr VectorBase(T v) : x{v}, y{v}, z{v}, w{v} {}

  // Transpose ctor
  constexpr VectorBase(VectorBase<4, T, flip(_orient)> const & other)
    : x{other.x}, y{other.y}, z{other.z}, w{other.w} {}

  template <std::size_t n>
  constexpr T get() const {
    static_assert(n < 4, "vector element out of range");
    return n == 0 ? x
                  : n == 1 ? y
                           : n == 2 ? z
                                    : w;
  }

  template <std::size_t n>
  T & get() {
    static_assert(n < 4, "vector element out of range");
    return n == 0 ? x
                  : n == 1 ? y
                           : n == 2 ? z
                                    : w;
  }
};

}  // namespace vec

/*******************************************************************************
 * Vector is the actual vector type, derived from VectorBase.
 *
 * Most C++ operators are implemented as component-wise operations, meaning that
 * '*' is not a vector product, but rather a vector of scalar products.  Vector
 * specific operations are free functions defined below.
 */

template <std::size_t _dim, typename T, Orient _orient>
struct Vector : public _vec::VectorBase<_dim, T, _orient> {
  static constexpr std::size_t dim = _dim;
  static constexpr Orient orient = _orient;

  using Transposed = Vector<_dim, T, flip(_orient)>;
  using Element = T;
  using Base = _vec::VectorBase<_dim, T, _orient>;

  constexpr Vector(Vector const &) = default;

  using _vec::VectorBase<_dim, T, _orient>::VectorBase;

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
constexpr T get(Vector<dim, T, orient> const & v) {
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

template <typename V>
constexpr auto transpose(V const & v) -> typename V::Transposed {
  return typename V::Transposed{v};
}

template <typename V>
inline constexpr auto dot(V const & a, V const & b) -> typename V::Element {
  using E = typename V::Element;
  return horizontal(a * b, functor::Add<E, E>{});
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
