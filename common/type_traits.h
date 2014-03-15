#ifndef ETL_COMMON_TYPE_TRAITS_H_
#define ETL_COMMON_TYPE_TRAITS_H_

/*
 * Analog of the C++11 type_traits header.
 */

#include "etl/common/attribute_macros.h"

namespace etl {
namespace common {

/*******************************************************************************
 * Promoting values to types
 */

/*
 * IntegralConstant creates types holding a constant value.
 */
template <typename T, T V>
struct IntegralConstant {
  static constexpr T value = V;
  typedef T value_type;
  typedef IntegralConstant<T, V> type;
  constexpr operator T() { return V; }
};

// Shorthand for the common case of IntegralConstant<bool, x>
template <bool V>
struct BoolConstant : public IntegralConstant<bool, V> {};

// Further shorthand for the two famous bool values.
typedef BoolConstant<true>  TrueType;
typedef BoolConstant<false> FalseType;


/*******************************************************************************
 * Promoting types to values.
 */

/*
 * TypeConstant creates a type of trivial values associated with a given type.
 * This can be used to pass a placeholder for a type in a value context,
 * without any overhead that might be associated with creating an actual
 * instance of the type (or even a pointer).
 */
template <typename T>
struct TypeConstant {
  typedef T type;
};


/*******************************************************************************
 * Basic operations on types
 */

/*
 * Equivalent of the C ternary operator, but over types.  The expression
 * Conditional<x, A, B>::type evaluates to A if x is true, B if x is false.
 */
template <bool, typename A, typename B>
struct Conditional {
  typedef A type;
};

// Partial specialization for false.
template <typename A, typename B>
struct Conditional<false, A, B> {
  typedef B type;
};

/*
 * The IsSame predicate tests if two types are exactly identical.
 */
template <typename T, typename S>
struct IsSame : public FalseType {};

template <typename T>
struct IsSame<T, T> : public TrueType {};


/*
 * RemoveReference<T>::type strips any reference type from T.
 */

template <typename T>
struct RemoveReference {
  typedef T type;
};

template <typename T>
struct RemoveReference<T &> {
  typedef T type;
};

template <typename T>
struct RemoveReference<T &&> {
  typedef T type;
};

/*
 * RemoveVolatile and RemoveConst strip qualifiers from a type.
 * RemoveConstVolatile does both.
 */
template <typename T>
struct RemoveVolatile {
  typedef T type;
};

template <typename T>
struct RemoveVolatile<T volatile> {
  typedef T type;
};

template <typename T>
struct RemoveConst {
  typedef T type;
};

template <typename T>
struct RemoveConst<T const> {
  typedef T type;
};

template <typename T>
struct RemoveConstVolatile {
 private:
  typedef typename RemoveConst<T>::type no_const;
 public:
  typedef typename RemoveVolatile<no_const>::type type;
};


/*******************************************************************************
 * Operations on C++ fundamental types.
 */

#define ETL_COMMON_SPECIALIZE(tmpl, type, value) \
  template <> \
  struct tmpl<type> : public BoolConstant<value> {}

/*
 * The IsUnqualifiedIntegral predicate matches fundamental integral types
 * without const/volatile qualifiers.  To match qualified types, see IsIntegral.
 */
template <typename T>
struct IsUnqualifiedIntegral : public FalseType {};

// Specializations for every built-in integral type.
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, bool, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, char, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, signed char, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, unsigned char, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, short, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, unsigned short, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, int, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, unsigned int, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, long, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, unsigned long, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, long long, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedIntegral, unsigned long long, true);

/*
 * The IsIntegral predicate matches fundamental integral types, even when
 * qualified with const/volatile.
 */
template <typename T>
struct IsIntegral
  : public IsUnqualifiedIntegral<typename RemoveConstVolatile<T>::type> {};

/*
 * The IsUnqualifiedFloatingPoint matches fundamental floating point types
 * without const/volatile qualifiers.
 */
template <typename T>
struct IsUnqualifiedFloatingPoint : public FalseType {};

// Specializations for built-in floating point types.
ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, float, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, double, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, long double, true);

/*
 * The IsFloatingPoint predicate matches fundamental floating point types,
 * even when qualified with const/volatile.
 */
template <typename T>
struct IsFloatingPoint
  : public IsUnqualifiedFloatingPoint<typename RemoveConstVolatile<T>::type> {};

/*
 * The IsArithmetic predicate matches fundamental arithmetic types, meaning
 * integral or floating point types.
 */
template <typename T>
struct IsArithmetic
  : public BoolConstant<IsIntegral<T>::value || IsFloatingPoint<T>::value> {};

/*
 * The IsSigned predicate matches arithmetic types that can represent negative
 * numbers.
 */
template <typename T>
struct IsSigned
  : public BoolConstant<IsFloatingPoint<T>::value
                        || (IsIntegral<T>::value && (T(-1) < T(0)))> {};

/*
 * The IsUnsigned predicate matches arithmetic types that cannot represent
 * negative numbers.
 */
template <typename T>
struct IsUnsigned
  : public BoolConstant<IsArithmetic<T>::value && !IsSigned<T>::value> {};

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_TYPE_TRAITS_H_
