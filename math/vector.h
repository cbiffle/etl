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
 * using `transposed(v)`.
 *
 *
 * Unit Vectors
 * ============
 *
 * Some operations are slightly cheaper when applied to vectors of unit length,
 * but only if the author or compiler can verify that at compile time.  To help
 * with this, this library distinguishes between the general `Vector` template
 * and the `UnitVector` subtype.
 *
 * You can get a `UnitVector` from `normalized(v)`, or by creating your own
 * with `from_arbitrary` -- effectively asserting to the compiler that the
 * vector is unit-length.
 *
 * A `UnitVector` is a `Vector`, so it can be used with any vector operation.
 *
 * Vector unit status is preserved across normalization and negation.  Other
 * operations will return a non-unit `Vector`.
 *
 *
 * Operations on Vectors
 * =====================
 *
 * This discussion refers to some vector type `V`.  I'll get more specific only
 * when required.
 *
 * Constexpr
 * ---------
 *
 * This library has been designed with constexpr in mind.  All vector operations
 * that don't require mutation are declared along the C++11 constexpr rules.
 *
 * Note that this means that operations over the elements of a vector are
 * unrolled at compile time.  For small vectors, this is almost always what
 * you want.  For large vectors of, say, 16 elements and up, this may not be
 * the right library.
 *
 * Whether the vector operations can actually be used in a constexpr context
 * depends on your compiler and the operations you use.  For example,
 * normalizing a vector in a constexpr context requires a constexpr `sqrt`
 * defined for the vector's element type.  C++11 explicitly defines `sqrt` as
 * non-constexpr; GCC (with `std=gnu++11` or later) allows `sqrt` in constexpr
 * contexts, but Clang (3.6.1) does not, even with GNU extensions enabled.
 *
 * When in doubt, test.
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
 * The `transposed(v)` function can also be used to derive a row vector from a
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
 * - `v1 * s` and `s * v1` are element-wise multiplication by scalars.
 * - `v1 / s` and `s / v1` are element-wise division by scalars.
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
 * - `transposed(v)` is a vector with the same elements as `v` but opposed
 *   orientation.
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

#include <cmath>

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
constexpr Orient flip(Orient o) {
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
 * IMPLEMENTATION DETAIL SECTION - NOT PUBLIC API
 *
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

  template <typename S>
  using WithType = Vector<_dim, S, _orient>;

  constexpr Vector() = default;
  constexpr Vector(Vector const &) = default;

  using Base::Base;

  /*
   * Conversion from vector of S to vector of T when size and orientation are
   * the same.
   */
  template <typename S>
  constexpr explicit Vector(Vector<_dim, S, _orient> const & other)
    : Vector(parallel(other, functor::Construct<S, T>{})) {}

  template <std::size_t I>
  constexpr T get() const {
    return this->Base::template get<I>();
  }

  /*
   * Shuffle accessor.
   */
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

/*
 * Element accessor function; alternative to the member template.
 */
template <std::size_t I, std::size_t dim, typename T, Orient orient>
constexpr auto get(Vector<dim, T, orient> const & v)
    -> typename std::enable_if<I < dim, T>::type {
  return v.template get<I>();
}

/*
 * Element shuffle function; alternative to the member template.
 */
template <std::size_t I0, std::size_t I1, std::size_t... I,
          std::size_t dim, typename T, Orient orient>
constexpr auto get(Vector<dim, T, orient> const & v)
      -> Vector<sizeof...(I) + 2, T, orient> {
  return v.template get<I0, I1, I...>();
}



/*******************************************************************************
 * IMPLEMENTATION DETAIL SECTION - NOT PUBLIC API
 *
 * Helper factors for combinators (below).
 */

namespace _vec {  // implementation details

  /*
   * Apply `f` to elements of `v` named by the indexes in the given
   * IndexSequence specialization (`I...`).  Note that this is significantly
   * more general than the public `parallel` -- it can perform arbitrary
   * shuffles.  For now I'm not taking advantage of this.
   */
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
      -> Vector<sizeof...(I), decltype(fn(T{})), orient> {
    return { fn(get<I>(v))... };
  }

  /*
   * Apply `f` to corresponding pairs of elements of `a` and `b` named by the
   * indexes in the given IndexSequence specialization (`I...`).  Note that
   * this is significantly more general than the public `parallel` -- it can
   * perform arbitrary shuffles.  For now I'm not taking advantage of this.
   */
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
      -> Vector<sizeof...(I), decltype(fn(T{}, S{})), orient> {
    return { fn(get<I>(a), get<I>(b))... };
  }

  /*
   * Horizontal combinator helper -- 1+ element case.
   *
   * In Haskell terms this is `foldl`.
   *
   * Note that this will have compile-time recursion depth proportional to the
   * dimension of the vectors.  Vectors are expected to be small, so this is
   * probably fine.
   */
  template <
    std::size_t dim,
    typename T,
    Orient orient,
    typename F,
    std::size_t I0,
    std::size_t... I
  >
  constexpr T horizontal_(Vector<dim, T, orient> const & v,
                          T accum,
                          F && fn,
                          IndexSequence<I0, I...>) {
    return horizontal_(v,
                       fn(accum, get<I0>(v)),
                       forward<F>(fn),
                       IndexSequence<I...>{});
  }

  /*
   * Horizontal combinator helper -- base case.
   */
  template <
    std::size_t dim,
    typename T,
    Orient orient,
    typename F
  >
  constexpr T horizontal_(Vector<dim, T, orient> const &,
                          T accum,
                          F &&,
                          IndexSequence<>) {
    return accum;
  }

}  // namespace _vec


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

/*
 * Produces a new vector by applying 'fn' to each element of 'v'.
 *
 *   auto a = Vec3f{1, 2, 3};
 *   auto b = parallel(a, [](float x) { return x + 2; });
 *   // b = Vec3f{3, 4, 5}
 */
template <
  std::size_t d,
  typename T,
  Orient orient,
  typename F
>
constexpr auto parallel(Vector<d, T, orient> const & v, F && fn)
    -> decltype(_vec::parallel_(v, forward<F>(fn), MakeIndexSequence<d>{})) {
  return _vec::parallel_(v, forward<F>(fn), MakeIndexSequence<d>{});
}

/*
 * Produces a new vector by applying 'fn' to corresponding elements of 'a' and
 * 'b'.
 *
 *   auto a = Vec3f{1, 2, 3};
 *   auto b = Vec3f{4, 5, 6};
 *   auto c = parallel(a, b, [](float x, float y) { return x + y; });
 *   // c = Vec3f{5, 7, 9}
 */
template <
  std::size_t d,
  typename T,
  typename S,
  Orient orient,
  typename F
>
constexpr auto parallel(Vector<d, T, orient> const & a,
                        Vector<d, S, orient> const & b,
                        F && fn)
    -> decltype(_vec::parallel_(a, b, forward<F>(fn), MakeIndexSequence<d>{})) {
  return _vec::parallel_(a, b, forward<F>(fn), MakeIndexSequence<d>{});
}

/*
 * Applies 'fn' to pairs of elements in 'v' until all elements have been
 * considered, and returns the result.
 *
 * `fn` is treated as left-associative, that is, applying `fn` to the vector
 * `{a, b, c}` produces `fn(fn(a, b), c)`, *not* `fn(a, fn(b, c))`.
 *
 *   auto a = Vec3f{1, 2, 3};
 *   auto b = parallel(a, [](float x, float y) { return x / y; });
 *   // b = 1.f / 6
 */
template <
  std::size_t d,
  typename T,
  Orient orient,
  typename F
>
constexpr auto horizontal(Vector<d, T, orient> const & v,
                          F && fn)
    -> decltype(fn(T{}, T{})) {
  return _vec::horizontal_(v,
                           get<0>(v),
                           forward<F>(fn),
                           MakeIndexSequence<d>::tail());
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

// Multiplication by scalar
template <
  std::size_t dim,
  typename T,
  typename S,
  Orient orient,
  typename = typename std::enable_if<!IsVector<T>::value>::type
>
constexpr auto operator*(T a, Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} * S{}), orient> {
  return parallel(b, functor::MultiplyValueBy<T, S>{a});
}

template <
  std::size_t dim,
  typename T,
  typename S,
  Orient orient,
  typename = typename std::enable_if<!IsVector<S>::value>::type
>
constexpr auto operator*(Vector<dim, T, orient> const & a, S b)
    -> Vector<dim, decltype(T{} * S{}), orient> {
  return parallel(a, functor::MultiplyByValue<T, S>{b});
}

// Division by scalar
template <
  std::size_t dim,
  typename T,
  typename S,
  Orient orient,
  typename = typename std::enable_if<!IsVector<S>::value>::type
>
constexpr auto operator/(Vector<dim, T, orient> const & a, S b)
    -> Vector<dim, decltype(T{} / S{}), orient> {
  return parallel(a, functor::DivideByValue<T, S>{b});
}

template <
  std::size_t dim,
  typename T,
  typename S,
  Orient orient,
  typename = typename std::enable_if<!IsVector<T>::value>::type
>
constexpr auto operator/(T a, Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} / S{}), orient> {
  return parallel(b, functor::DivideValueBy<T, S>{a});
}

/*******************************************************************************
 * Compound assignment.  These are derived from the binary operators using a
 * common pattern.
 */

template <std::size_t dim, typename T, typename S, Orient orient>
inline
Vector<dim, T, orient> & operator+=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target + other;
}

template <std::size_t dim, typename T, typename S, Orient orient>
inline
Vector<dim, T, orient> & operator-=(Vector<dim, T, orient> & target,
                                    Vector<dim, S, orient> const & other) {
  return target = target - other;
}

template <
  std::size_t dim,
  typename T,
  Orient orient,
  typename S,
  typename = typename std::enable_if<!IsVector<S>::value>::type
>
inline Vector<dim, T, orient> & operator*=(Vector<dim, T, orient> & target,
                                           S scalar) {
  return target = target * scalar;
}

template <
  std::size_t dim,
  typename T,
  Orient orient,
  typename S,
  typename = typename std::enable_if<!IsVector<S>::value>::type
>
inline Vector<dim, T, orient> & operator/=(Vector<dim, T, orient> & target,
                                           S scalar) {
  return target = target / scalar;
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
constexpr auto transposed(Vector<dim, T, orient> const & v)
    -> Vector<dim, T, flip(orient)> {
  return Vector<dim, T, flip(orient)>{v};
}

template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto parallel_mul(Vector<dim, T, orient> const & a,
                            Vector<dim, S, orient> const & b)
    -> Vector<dim, decltype(T{} * S{}), orient> {
  return parallel(a, b, functor::Multiply<T, S>{});
}

template <std::size_t dim, typename T, typename S, Orient orient>
constexpr auto dot(Vector<dim, T, orient> const & a,
                   Vector<dim, S, orient> const & b)
    -> decltype(T{} * S{}) {
  return horizontal(parallel_mul(a, b),
                    functor::Add<T, S>{});
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

template <typename T, typename S, Orient orient>
constexpr auto cross(Vector<3, T, orient> const & a,
                     Vector<3, S, orient> const & b)
    -> Vector<3, decltype(T{} * S{} - T{} * S{}), orient> {
  return parallel_mul(get<1, 2, 0>(a), get<2, 0, 1>(b))
       - parallel_mul(get<2, 0, 1>(a), get<1, 2, 0>(b));
}


/*******************************************************************************
 * Unit vector subtype.
 *
 * The usual way of obtaining a UnitVector is by calling `normalized(v)`.
 *
 * In general, a unit vector can be substituted for a vector, but most
 * operations on vectors don't preserve the norm and thus lose the result's unit
 * status.  Operations that preserve unit status are overloaded below.
 */

template <std::size_t _dim, typename T, Orient _orient>
struct UnitVector : public Vector<_dim, T, _orient> {
  using Base = Vector<_dim, T, _orient>;

  UnitVector() = delete;

  // It's safe to copy unit vectors.
  constexpr UnitVector(UnitVector const &) = default;

  static constexpr UnitVector from_arbitrary(Base const & v) {
    return UnitVector{v};
  }

private:
  constexpr explicit UnitVector(Base const & v) : Base(v) {}
};

template <std::size_t dim, typename T, Orient orient>
constexpr auto normalized(Vector<dim, T, orient> const & a)
    -> UnitVector<dim, T, orient> {
  return UnitVector<dim, T, orient>::from_arbitrary(a / norm(a));
}

template <std::size_t dim, typename T, Orient orient>
constexpr auto normalized(UnitVector<dim, T, orient> const & a)
    -> UnitVector<dim, T, orient> {
  return a;
}

template <
  std::size_t dim,
  typename T,
  Orient orient,
  typename R = UnitVector<dim, decltype(-T{}), orient>
>
constexpr R operator-(UnitVector<dim, T, orient> const & v) {
  return R::from_arbitrary(-Vector<dim, T, orient>{v});
}


/*******************************************************************************
 * Aliases for common vector types.
 */

template <typename T, Orient orient = Orient::col>
using Vec2 = Vector<2, T, orient>;

template <typename T, Orient orient = Orient::col>
using Vec3 = Vector<3, T, orient>;

template <typename T, Orient orient = Orient::col>
using Vec4 = Vector<4, T, orient>;

template <typename T, Orient orient = Orient::col>
using UVec2 = UnitVector<2, T, orient>;

template <typename T, Orient orient = Orient::col>
using UVec3 = UnitVector<3, T, orient>;

template <typename T, Orient orient = Orient::col>
using UVec4 = UnitVector<4, T, orient>;

using Vec2f = Vec2<float>;
using Vec3f = Vec3<float>;
using Vec4f = Vec4<float>;

using Vec2i = Vec2<int>;
using Vec3i = Vec3<int>;
using Vec4i = Vec4<int>;

using UVec2f = UVec2<float>;
using UVec3f = UVec3<float>;
using UVec4f = UVec4<float>;

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_VECTOR_H_INCLUDED
