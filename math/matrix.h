#ifndef _ETL_MATH_MATRIX_H_INCLUDED
#define _ETL_MATH_MATRIX_H_INCLUDED

/*
 * Matrix templates.
 *
 * A matrix is an `n` row by `m` column grid of scalar of type `T`.  We
 * represent this as the type `Matrix<n, m, T>`, a collection of `n` row
 * vectors, each with `m` elements.
 *
 * The discussion below will refer to some abstract matrix type `M` where
 * something more specific isn't necessary.
 *
 * Constructing
 * ============
 *
 * In addition to the normal copy and move constructors, matrices can be
 * constructed in the following ways:
 *
 * - Default: `M{}` default-constructs the scalar elements.  This is implicit
 *   and can often be written `{}`.
 * - Literal: specifying each element: `M{ {a, b, c...}, {d, e, f...} }`.
 * - Identity: if `M` is a square matrix type, `M::identity()`.
 *
 * Operations on matrices
 * ======================
 *
 * Matrices support multiplication by column vectors: `m * v`.
 *
 * Multiplication by row vectors (`v * m`) and multiplication by scalars are
 * not currently supported because I haven't needed it.
 *
 * Matrix transposition is written `transposed(m)`.
 */

#include <type_traits>

#include "etl/functor.h"
#include "etl/type_list.h"
#include "etl/math/vector.h"

namespace etl {
namespace math {

/*******************************************************************************
 * Fundamental Matrix types.
 */
  
// Common parent of all matrices.
struct MatrixTag {};

// Convenient alias for matrix rows.
template <std::size_t cols, typename T>
using MatrixRow = Vector<cols, T, Orient::row>;

/*
 * Base class of matrices of a given size and type.  Defines storage, type
 * aliases, and constructors so that we don't have to repeat them in the
 * specializations below.
 *
 * This base class is doing something slightly odd: it requires a TypeList
 * consisting of `rows` repetitions of its `Row` type, so that it can generate
 * a precise constructor.  To supply this, the specializations below inherit
 * indirectly through the `MatrixBaseHelper` template alias (below).
 */
template <
  std::size_t rows,
  std::size_t cols,
  typename T,
  typename TL>
struct MatrixBase;

// Slight specialization to capture the contents of the TypeList
template <
  std::size_t rows,
  std::size_t cols,
  typename T,
  typename... Rs>
struct MatrixBase<rows, cols, T, TypeList<Rs...>> : public MatrixTag {
  using Row = MatrixRow<cols, T>;

  Row row[rows];

  // Default ctor
  constexpr MatrixBase() = default;

  // Literal ctor
  constexpr MatrixBase(Rs const & ...args) : row{args...} {}
};

/*
 * Helpful template alias for choosing the right `MatrixBase` parent for a
 * matrix of a given size and type.  This takes care of generating the
 * TypeList used as constructor arguments.
 */
template <
  std::size_t rows,
  std::size_t cols,
  typename T
>
using MatrixBaseHelper = MatrixBase<rows, cols, T,
      Repeat<MatrixRow<cols, T>, rows>>;

/*
 * General Matrix template; used for rectangular matrices.
 */
template <std::size_t _rows, std::size_t _cols, typename T>
struct Matrix : public MatrixBaseHelper<_rows, _cols, T> {
  using Base = MatrixBaseHelper<_rows, _cols, T>;

  using Base::Base;

  static constexpr std::size_t rows = _rows;
  static constexpr std::size_t cols = _cols;
  static constexpr bool square = false;
  using Element = T;
  using typename Base::Row;
};

/*
 * Specialization for square matrices.
 */
template <std::size_t _side, typename T>
struct Matrix<_side, _side, T> : public MatrixBaseHelper<_side, _side, T> {
  using Base = MatrixBaseHelper<_side, _side, T>;

  using Base::Base;

  static constexpr std::size_t rows = _side;
  static constexpr std::size_t cols = _side;
  static constexpr bool square = true;
  using Element = T;
  using typename Base::Row;

  /*
   * Returns the identity matrix for this type.
   */
  static constexpr Matrix identity() {
    return identity_(MakeIndexSequence<_side>{});
  }

private:
  template <std::size_t... R>
  static constexpr Matrix identity_(IndexSequence<R...>) {
    return { identity_row<R>(MakeIndexSequence<_side>{})... };
  }

  template <std::size_t row, std::size_t... C>
  static constexpr Row identity_row(IndexSequence<C...>) {
    return { (row == C ? T{1} : T{0})... };
  }
};

/*
 * Aliases for common square matrices.
 */

template <typename T>
using Mat2 = Matrix<2, 2, T>;

template <typename T>
using Mat3 = Matrix<3, 3, T>;

template <typename T>
using Mat4 = Matrix<4, 4, T>;

using Mat2f = Mat2<float>;
using Mat3f = Mat3<float>;
using Mat4f = Mat4<float>;



/*******************************************************************************
 * Basic matrix operations.
 */

namespace _matrix {

  template <
    std::size_t r,
    std::size_t c,
    typename T,
    typename S,
    typename F,
    std::size_t... R
  >
  constexpr auto row_wise_(Matrix<r, c, T> const & a,
                           Matrix<r, c, S> const & b,
                           F && fn,
                           IndexSequence<R...>)
      -> Vector<r, decltype(fn(MatrixRow<c, T>{}, MatrixRow<c, S>{}))> {
    return { fn(a.row[R], b.row[R])... };
  }

  template <
    std::size_t r,
    std::size_t c,
    typename T,
    typename F,
    std::size_t... R
  >
  constexpr auto row_wise_(Matrix<r, c, T> const & a,
                           F && fn,
                           IndexSequence<R...>)
      -> Vector<r, decltype(fn(MatrixRow<c, T>{}))> {
    return { fn(a.row[R])... };
  }

  template <
    std::size_t col,
    std::size_t r,
    std::size_t c,
    typename T,
    std::size_t... R
  >
  constexpr auto transpose_row(Matrix<r, c, T> const & a,
                               IndexSequence<R...>)
      -> MatrixRow<r, T> {
    return { get<col>(a.row[R])... };
  }

  template <
    std::size_t r,
    std::size_t c,
    typename T,
    std::size_t... R,
    std::size_t... C
  >
  constexpr auto transpose_(Matrix<r, c, T> const & a,
                            IndexSequence<R...> rs,
                            IndexSequence<C...>)
      -> Matrix<c, r, T> {
    return { transpose_row<C>(a, rs)... };
  }

}  // namespace _matrix

/*
 * Row-wise binary combinator.  Applies the given functor to corresponding
 * pairs of rows in matrices 'a' and 'b', producing a column vector containing
 * the result for each row.
 */
template <
  std::size_t r,
  std::size_t c,
  typename T,
  typename S,
  typename F,
  typename R1 = typename Matrix<r, c, T>::Row,
  typename R2 = typename Matrix<r, c, S>::Row
>
constexpr auto row_wise(Matrix<r, c, T> const & a,
                        Matrix<r, c, S> const & b,
                        F && fn)
    -> Vector<r, decltype(fn(MatrixRow<c, T>{}, MatrixRow<c, S>{}))> {
  return _matrix::row_wise_(a, b, forward<F>(fn), MakeIndexSequence<r>{});
}

/*
 * Row-wise unary combinator.  Applies the given functor to each row in the
 * matrix, producing a column vector containing the result for each row.
 */
template <std::size_t r, std::size_t c, typename T, typename F>
constexpr auto row_wise(Matrix<r, c, T> const & a,
                        F && fn)
    -> Vector<r, decltype(fn(MatrixRow<c, T>{}))> {
  return _matrix::row_wise_(a, forward<F>(fn), MakeIndexSequence<r>{});
}

/*
 * Matrix equality.
 */
template <std::size_t r, std::size_t c, typename T, typename S>
constexpr bool operator==(Matrix<r, c, T> const & a,
                          Matrix<r, c, S> const & b) {
  using R1 = typename Matrix<r, c, T>::Row;
  using R2 = typename Matrix<r, c, S>::Row;
  return horizontal(row_wise(a, b, functor::Equal<R1, R2>{}),
                    functor::LogicalAnd<bool, bool>{});
}

/*
 * Matrix inequality.
 */
template <std::size_t r, std::size_t c, typename T, typename S>
constexpr bool operator!=(Matrix<r, c, T> const & a,
                          Matrix<r, c, S> const & b) {
  using R1 = typename Matrix<r, c, T>::Row;
  using R2 = typename Matrix<r, c, S>::Row;
  return horizontal(row_wise(a, b, functor::NotEqual<R1, R2>{}),
                    functor::LogicalOr<bool, bool>{});
}

/*
 * Matrix transposition.  Converts an n x m matrix into an m x n matrix by
 * flipping around the diagonal.
 */
template <std::size_t r, std::size_t c, typename T>
constexpr auto transposed(Matrix<r, c, T> const & m) -> Matrix<c, r, T> {
  return _matrix::transpose_(m, MakeIndexSequence<r>{}, MakeIndexSequence<c>{});
}

/*******************************************************************************
 * Matrix multiplication.
 */

namespace _matrix {

  /*
   * Matrix-by-vector multiplication helper.
   */
  template <std::size_t n, typename M, typename V, std::size_t... I>
  constexpr auto mul_m_v(Matrix<n, n, M> const & m,
                         Vector<n, V> const & v,
                         IndexSequence<I...>)
      -> Vector<n, decltype(M{} * V{})> {
    return { dot(transposed(m.row[I]), v)... };
  }

  /*
   * Matrix multiplication helper, leaf level.
   *
   * This computes a single element of the final matrix.
   *
   * This could likely be rewritten in terms of dot product.
   */
  template <
    std::size_t row,
    std::size_t col,
    std::size_t n,
    std::size_t m,
    std::size_t p,
    typename T1,
    typename T2,
    std::size_t... M>
  constexpr auto mul_m_m_1(Matrix<n, m, T1> const & a,
                           Matrix<m, p, T2> const & b,
                           IndexSequence<M...>)
      -> decltype(T1{} * T2{}) {
    using R = decltype(T1{} * T2{});
    using V = Vector<m, R>;
    return horizontal(V{(get<M>(a.row[row]) * get<col>(b.row[M]))...},
                      functor::Add<T1, T2>{});
  }

  /*
   * Matrix multiplication helper, mid level.
   *
   * This expands into a literal vector with each element generated by the
   * next level helper, mul_m_m_1, above.
   */
  template <
    std::size_t row,
    std::size_t n,
    std::size_t m,
    std::size_t p,
    typename T1,
    typename T2,
    std::size_t... M,
    std::size_t... P>
  constexpr auto mul_m_m_row(Matrix<n, m, T1> const & a,
                             Matrix<m, p, T2> const & b,
                             IndexSequence<M...> ms,
                             IndexSequence<P...>)
      -> MatrixRow<p, decltype(T1{} * T2{})> {
    return { mul_m_m_1<row, P>(a, b, ms)... };
  }

  /*
   * Matrix multiplication helper, top level.
   *
   * This expands into a literal matrix where each row is generated by
   * the next level helper, mul_m_m_row, above.
   */
  template <
    std::size_t n,
    std::size_t m,
    std::size_t p,
    typename T1,
    typename T2,
    std::size_t... N,
    std::size_t... M,
    std::size_t... P
  >
  constexpr auto mul_m_m(Matrix<n, m, T1> const & a,
                         Matrix<m, p, T2> const & b,
                         IndexSequence<N...>,
                         IndexSequence<M...> ms,
                         IndexSequence<P...> ps)
      -> Matrix<n, p, decltype(T1{} * T2{})> {
    return { mul_m_m_row<N>(a, b, ms, ps)... };
  }

}  // namespace _matrix


/*
 * Multiplication of a matrix by a column vector.
 */
template <std::size_t n, typename M, typename V>
constexpr auto operator*(Matrix<n, n, M> const & m,
                         Vector<n, V> const & v)
    -> Vector<n, decltype(M{} * V{})> {
  return _matrix::mul_m_v(m, v, MakeIndexSequence<n>{});
}

/*
 * Multiplication of a matrix by another matrix.
 */
template <
  std::size_t n,
  std::size_t m,
  typename T1,
  std::size_t p,
  typename T2
>
constexpr auto operator*(Matrix<n, m, T1> const & a,
                         Matrix<m, p, T2> const & b)
    -> Matrix<n, p, decltype(T1{} * T2{})> {
  return _matrix::mul_m_m(a, b,
      MakeIndexSequence<n>{},
      MakeIndexSequence<m>{},
      MakeIndexSequence<p>{});
}

}  // namespace math
}  // namespace etl

#endif  // _ETL_MATH_MATRIX_H_INCLUDED
