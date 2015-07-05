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

  constexpr explicit VectorBase(T val) : elements{[0 ... (dim-1)] = val} {}

  // Type/orientation conversion ctor
  template <typename S, Orient o2>
  explicit constexpr VectorBase(VectorBase<dim, S, o2> const & other)
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
};


/*
 * Specialization of VectorBase for 2-vectors with fields named x, y.
 */
template <typename T, Orient _orient>
struct VectorBase<2, T, _orient> : public VectorTag {
  T x, y;

  constexpr VectorBase() = default;
  constexpr VectorBase(T x_, T y_) : x{x_}, y{y_} {}
  constexpr VectorBase(T v) : x{v}, y{v} {}

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

  constexpr VectorBase() = default;

  template <typename S, Orient o2>
  explicit constexpr VectorBase(VectorBase<3, S, o2> const & other)
    : x{other.x}, y{other.y}, z{other.z} {}

  constexpr VectorBase(T x_, T y_, T z_) : x{x_}, y{y_}, z{z_} {}
  constexpr VectorBase(T v) : x{v}, y{v}, z{v} {}

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

  constexpr VectorBase() = default;
  constexpr VectorBase(T x_, T y_, T z_, T w_)
    : x{x_}, y{y_}, z{z_}, w{w_} {}
  constexpr VectorBase(T v) : x{v}, y{v}, z{v}, w{v} {}

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

  using This = Vector<_dim, T, _orient>;
  using Transposed = Vector<_dim, T, flip(_orient)>;
  using Element = T;

  constexpr Vector(Vector const &) = default;

  using _vec::VectorBase<_dim, T, _orient>::VectorBase;

  template <typename S>
  constexpr explicit Vector(Vector<_dim, S, _orient> const & other)
    : Vector(other.unary([] (S x) { return T(x); })) {}

  template <typename S>
  using WithType = Vector<_dim, S, _orient>;

  template <std::size_t I>
  constexpr T get() const {
    return this->VectorBase::template get<I>();
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

  /********************************************************************
   * Lift combinators
   */

  template <typename F>
  constexpr auto unary(F && fn) const
      -> WithType<decltype(fn(T{}))> {
    return unary_(forward<F>(fn), MakeIndexSequence<dim>{});
  }

  template <typename F, typename S>
  constexpr auto binary(WithType<S> const & other, F && fn) const
      -> WithType<decltype(fn(T{}, S{}))> {
    return binary_(other, forward<F>(fn), MakeIndexSequence<dim>{});
  }

  template <typename F>
  constexpr auto reduce(F && fn) const -> T {
    return reduce_(forward<F>(fn), MakeIndexSequence<dim>{});
  }

  /********************************************************************
   * Lifted operators
   */

  constexpr This operator-() const {
    return unary(functor::Negate<T>{});
  }

  template <typename S>
  constexpr This operator+(WithType<S> const & other) const {
    return binary(other, functor::Add<T, S>{});
  }

  template <typename S>
  constexpr This operator-(WithType<S> const & other) const {
    return binary(other, functor::Subtract<T, S>{});
  }

  template <typename S>
  constexpr This operator*(WithType<S> const & other) const {
    return binary(other, functor::Multiply<T, S>{});
  }

  template <typename S>
  constexpr This operator/(WithType<S> const & other) const {
    return binary(other, functor::Divide<T, S>{});
  }


  /********************************************************************
   * Compound assignment
   */

  template <typename S>
  This & operator+=(WithType<S> const & other) {
    *this = *this + other;
    return *this;
  }

  template <typename S>
  This & operator-=(WithType<S> const & other) {
    *this = *this - other;
    return *this;
  }

  template <typename S>
  This & operator*=(WithType<S> const & other) {
    *this = *this * other;
    return *this;
  }

  template <typename S>
  This & operator/=(WithType<S> const & other) {
    *this = *this / other;
    return *this;
  }


  /********************************************************************
   * Comparison
   */

  template <typename S>
  constexpr bool operator==(WithType<S> const & other) const {
    return binary(other, functor::Equal<T, S>{})
          .reduce(functor::LogicalAnd<bool, bool>{});
  }

  template <typename S>
  constexpr bool operator!=(WithType<S> const & other) const {
    return binary(other, functor::NotEqual<T, S>{})
          .reduce(functor::LogicalOr<bool, bool>{});
  }

private:
  template <typename F, std::size_t... I>
  constexpr auto unary_(F && fn, IndexSequence<I...>) const
      -> WithType<decltype(fn(T{}))> {
    return { fn(this->template get<I>())... };
  }

  template <typename F, typename S, std::size_t... I>
  constexpr auto binary_(WithType<S> const & other,
                         F && fn,
                         IndexSequence<I...>) const
      -> WithType<decltype(fn(T{}, S{}))> {
    return { fn(this->template get<I>(), other.template get<I>())... };
  }

  template <typename F, std::size_t I0, std::size_t I1, std::size_t... I>
  constexpr T reduce_(F && fn, IndexSequence<I0, I1, I...>) const {
    return fn(this->template get<I0>(),
              reduce_(forward<F>(fn), IndexSequence<I1, I...>{}));
  }

  template <typename F, std::size_t I0>
  constexpr T reduce_(F && fn, IndexSequence<I0>) const {
    return this->template get<I0>();
  }
};


/*******************************************************************************
 * Mixed vector-scalar operations.
 */

/*
template <
  typename V,
  typename S,
  typename = std::enable_if<!IsVector<S>::value>,
  typename E = typename V::Element,
  typename R = Vector<V::dim, decltype(E{} + S{}), V::orient>
>
inline constexpr R operator+(V const & v, S const & s) {
  return v.unary([&](E e) { return e + s; });
}

template <typename V, typename S>
inline constexpr auto operator*(V const & v, S const & s)
    -> typename std::enable_if<!std::is_base_of<VectorTag, S>::value,
                               decltype(typename V::Element{} * S{})>::type {
  return v.unary([&](typename V::Element e) { return e * s; });
}

template <typename V, typename S>
inline constexpr auto operator*(S const & s, V const & v)
    -> typename std::enable_if<!std::is_base_of<VectorTag, S>::value,
                               decltype(S{} * typename V::Element{})>::type {
  return v.unary([&](typename V::Element e) { return s * e; });
}
*/


/*******************************************************************************
 * Vector-specific operations implemented as non-member functions.
 */

template <typename V>
constexpr auto transpose(V const & v) -> typename V::Transposed {
  return typename V::Transposed{v};
}

template <typename V>
inline constexpr auto dot(V const & a, V const & b) -> typename V::Element {
  using E = typename V::Element;
  return (a * b).reduce(functor::Add<E, E>{});
}

template <typename T>
using Vec2 = Vector<2, T, Orient::col>;

template <typename T>
using Vec3 = Vector<3, T, Orient::col>;

template <typename T>
using Vec4 = Vector<4, T, Orient::col>;

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_VECTOR_H_INCLUDED
