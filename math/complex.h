#ifndef _ETL_MATH_COMPLEX_H_INCLUDED
#define _ETL_MATH_COMPLEX_H_INCLUDED

/*
 * Complex number support.
 *
 * Yes, C++ already has a lovely complex number type.  Unfortunately it isn't
 * part of the 'freestanding' support level that ETL targets.  If your
 * application can make use of the standard complex type, it probably should.
 *
 * When ETL is compiled on a hosted (non-freestanding) C++ implementation,
 * this Complex type gains implicit constexpr conversions to and from
 * std::complex.
 *
 * The names of complex number operations are chosen to match those used for
 * `std::complex`.  In cases where this introduces asymmetry with the rest of
 * ETL, multiple names are provided, e.g. both `abs(c)` and `mag(c)`.
 */

#if __STDC_HOSTED__
#  include <complex>
#endif

#include <type_traits>

#include "etl/math/signum.h"
#include "etl/math/vector.h"

namespace etl {
namespace math {

/*
 * Tag type for the family of types `Complex<T>`, so they can be recognized
 * at the type level.
 */
struct ComplexTag {};

/*
 * Predicate template for recognizing if a type `C` is a member of the family
 * `Complex<T>`.
 */
template <typename C>
using IsComplex = std::is_base_of<ComplexTag, typename std::decay<C>::type>;


/*******************************************************************************
 * Complex template, representing a complex number constructed from a real and
 * imaginary part, both of type `T`.
 */

template <typename T>
class Complex : public ComplexTag {
  static_assert(!IsComplex<T>::value, "Cannot create Complex<Complex<T>>");
public:
  using Value = T;

  /*
   * Building a complex number from its real part and imaginary coefficient.
   *
   * The imaginary term can be omitted to produce the complex equivalent of a
   * real.
   *
   * Both terms can be omitted to produce complex zero.
   */
  constexpr Complex(T real = T{}, T imag = T{}) : _real{real}, _imag{imag} {}

  /*
   * Implicit conversion constructor.  If a value of type `S` can be assigned
   * to type `T` with implicit conversion, the same is true of types
   * `Complex<S>` and `Complex<T>`.
   */
  template <
    typename S,
    typename = typename std::enable_if<std::is_assignable<T, S>::value>::type
  >
  constexpr Complex(Complex<S> const & other)
    : _real{other.real()}, _imag{other.imag()} {}

  /*
   * Explicit conversion constructor.  If a value of type `S` can be explicitly
   * converted to type `T`, the same is true of types `Complex<S>` and
   * `Complex<T>`.
   */
  template <typename S>
  constexpr explicit Complex(Complex<S> const & other)
    : _real{T(other.real())}, _imag{T(other.imag())} {}

  /*
   * Accessor for the real part.  See also the function `real(c)`.
   */
  constexpr T real() const { return _real; }

  /*
   * Accessor for the imaginary coefficient.  See also the function `imag(c)`.
   */
  constexpr T imag() const { return _imag; }

#if __STDC_HOSTED__ == 1
  /******************************************************************
   * Operations available only in hosted environments, for interop
   * with <complex>.
   */

  constexpr Complex(std::complex<T> const & c)
    : _real{c.real()}, _imag{c.imag()} {}

  constexpr operator std::complex<T>() const {
    return {_real, _imag};
  }
#endif

private:
  T _real;
  T _imag;
};

/*
 * Factory for Complex with type inference.
 */
template <typename T>
constexpr Complex<T> complex(T r, T i = {}) {
  return {r, i};
}

/*
 * Accessor for the real part.  See also member function `c.real()`.
 */
template <typename T>
constexpr T real(Complex<T> const & c) {
  return c.real();
}

/*
 * Accessor for the imaginary coefficient.  See also member function `c.imag()`.
 */
template <typename T>
constexpr T imag(Complex<T> const & c) {
  return c.imag();
}

/*
 * Utility for treating a 2-vector as a point in the complex plane.  Used below
 * to lift vector operations to operations over complex numbers.
 */
template <typename T>
constexpr Complex<T> make_complex(Vec2<T> const & v) {
  return {v.x, v.y};
}

/*
 * Utility for treating a complex number as a point in R^2.  Used below to lift
 * vector operations to operations over complex numbers.
 */
template <typename T>
constexpr Vec2<T> vec2(Complex<T> const & v) {
  return {real(v), imag(v)};
}

/*
 * The complex conjugate of `c`.
 */
template <typename T>
constexpr auto conjugate(Complex<T> const & c)
    -> Complex<decltype(-T{})> {
  return {real(c), -imag(c)};
}

/*
 * Alternative way of writing `conjugate(c)` for symmetry with the standard
 * library.
 */
template <typename T>
constexpr auto conj(Complex<T> const & c) -> decltype(conjugate(c)) {
  return conjugate(c);
}

/*
 * The magnitude (absolute value, modulus) of `c`.
 */
template <typename T>
constexpr auto mag(Complex<T> const & c) -> decltype(mag(vec2(c))) {
  return mag(vec2(c));
}

/*
 * Alternate way of writing `mag(c)` for symmetry with the standard library.
 */
template <typename T>
constexpr auto abs(Complex<T> const & c) -> decltype(mag(c)) {
  return mag(c);
}

/*
 * The square of the magnitude (absolute value, modulus) of `c`.
 */
template <typename T>
constexpr auto sqmag(Complex<T> const & c) -> decltype(sqmag(vec2(c))) {
  return sqmag(vec2(c));
}


/*******************************************************************************
 * Equality for complex numbers.
 *
 * Overloads are provided for complex-scalar inequality.
 */

/*
 * Complex equality.
 */
template <typename T, typename S>
constexpr bool operator==(Complex<T> const & a, Complex<S> const & b) {
  return vec2(a) == vec2(b);
}

/*
 * Complex-scalar equality.
 */
template <
  typename T,
  typename S,
  typename = std::enable_if<!IsComplex<S>::value>
>
constexpr bool operator==(Complex<T> const & a, S const & b) {
  return a == Complex<S>{b};
}

template <
  typename T,
  typename S,
  typename = std::enable_if<!IsComplex<T>::value>
>
constexpr bool operator==(T const & a, Complex<S> const & b) {
  return Complex<T>{a} == b;
}

/*
 * Complex inequality.
 */
template <typename T, typename S>
constexpr bool operator!=(Complex<T> const & a, Complex<S> const & b) {
  return vec2(a) != vec2(b);
}

/*
 * Complex-scalar inequality.
 */
template <
  typename T,
  typename S,
  typename = std::enable_if<!IsComplex<S>::value>
>
constexpr bool operator!=(Complex<T> const & a, S const & b) {
  return a != Complex<S>{b};
}

template <
  typename T,
  typename S,
  typename = std::enable_if<!IsComplex<T>::value>
>
constexpr bool operator!=(T const & a, Complex<S> const & b) {
  return Complex<T>{a} != b;
}


/*******************************************************************************
 * Arithmetic for complex numbers.
 *
 * Overloads are provided for complex-scalar operations.
 *
 * TODO: but there should be more.
 */

/*
 * Complex negation.
 */
template <typename T>
constexpr Complex<T> operator-(Complex<T> const & c) {
  return make_complex(-vec2(c));
}

/*
 * Complex addition.
 */
template <typename T, typename S>
constexpr auto operator+(Complex<T> const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return make_complex(vec2(a) + vec2(b));
}

/*
 * Complex subtraction.
 */
template <typename T, typename S>
constexpr auto operator-(Complex<T> const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return make_complex(vec2(a) - vec2(b));
}

/*
 * Complex multiplication.
 */
template <typename T, typename S>
constexpr auto operator*(Complex<T> const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return {
    real(a) * real(b) - imag(a) * imag(b),
    real(a) * imag(b) + imag(a) * real(b),
  };
}

/*
 * Complex multiplication by scalar.
 */
template <
  typename T,
  typename S,
  typename = typename std::enable_if<!IsComplex<S>::value>
>
constexpr auto operator*(Complex<T> const & a, S const & b)
    -> Complex<CommonType<T, S>> {
  return a * complex(b);
}

/*
 * Complex multiplication by scalar, the other way 'round.
 */
template <
  typename T,
  typename S,
  typename = typename std::enable_if<!IsComplex<T>::value>
>
constexpr auto operator*(T const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return complex(a) * b;
}

/*
 * Complex division.
 */
template <typename T, typename S>
constexpr auto operator/(Complex<T> const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return make_complex(vec2(a * conjugate(b)) / sqmag(b));
}

/*
 * Complex division by scalar.
 */
template <
  typename T,
  typename S,
  typename = typename std::enable_if<!IsComplex<S>::value>
>
constexpr auto operator/(Complex<T> const & a, S const & b)
    -> Complex<CommonType<T, S>> {
  return a / complex(b);
}

/*
 * Complex division *of* scalar.
 */
template <
  typename T,
  typename S,
  typename = typename std::enable_if<!IsComplex<T>::value>
>
constexpr auto operator/(T const & a, Complex<S> const & b)
    -> Complex<CommonType<T, S>> {
  return complex(a) / b;
}

/*
 * Complex square root.
 *
 * The return type for this assumes that `sqrt` itself does not promote `T`
 * in any way that multiplication would not.  This is a concession to the
 * lookup performed by `decltype`, where we cannot `using namespace`.
 */
template <typename T>
constexpr auto sqrt(Complex<T> const & c) -> Complex<decltype(T{} * T{})> {
  using namespace std;
  return {
    sqrt((real(c) + mag(c)) / T{2}),
    sgn(imag(c)) * sqrt((-real(c) + mag(c)) / 2)
  };
}


/*******************************************************************************
 * Compound assignment operators for complex numbers.
 */

template <typename T, typename S>
inline Complex<T> & operator+=(Complex<T> const & target,
                               Complex<S> const & c) {
  return target = target + c;
}

template <typename T, typename S>
inline Complex<T> & operator-=(Complex<T> & target,
                               Complex<S> const & c) {
  return target = target - c;
}

template <typename T, typename S>
inline Complex<T> & operator*=(Complex<T> & target,
                               Complex<S> const & c) {
  return target = target * c;
}

template <typename T, typename S>
inline Complex<T> & operator/=(Complex<T> & target,
                               Complex<S> const & c) {
  return target = target / c;
}


/*******************************************************************************
 * Unit complex numbers.  Statically distinguishing when a complex number is
 * known to be of unit magnitude can allow for certain optimizations.
 *
 * A `UnitComplex<T>` is-a `Complex<T>` and can be substituted, but doing so
 * strips its unit status.  Thus we overload certain operations for unit
 * complex numbers below, in cases where we can statically ensure that the
 * magnitude is preserved.
 */

template <typename T>
using UnitComplex = Unit<Complex<T>>;

/*
 * Unit negation; preserves magnitude.
 */
template <typename T>
constexpr UnitComplex<T> operator-(UnitComplex<T> const & c) {
  return unit_unchecked(-as_nonunit(c));
}

/*
 * Unit-by-unit multiplication; preserves magnitude.
 */
template <typename T, typename S>
constexpr auto operator*(UnitComplex<T> const & a, UnitComplex<S> const & b)
    -> Unit<decltype(as_nonunit(a) * as_nonunit(b))> {
  return unit_unchecked(as_nonunit(a) * as_nonunit(b));
}

/*
 * Unit-unit compound multiplication-assignment.
 */
template <typename T, typename S>
inline UnitComplex<T> & operator*=(UnitComplex<T> & target,
                                   UnitComplex<S> const & c) {
  return target = target * c;
}

/*
 * Unit square root.
 */
template <typename T>
constexpr auto sqrt(UnitComplex<T> const & c)
    -> Unit<decltype(sqrt(as_nonunit(c)))> {
  return unit_unchecked(sqrt(as_nonunit(c)));
}

}  // namespace etl
}  // namespace math

#endif  // _ETL_MATH_COMPLEX_H_INCLUDED
