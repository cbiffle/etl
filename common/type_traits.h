#ifndef ETL_COMMON_TYPE_TRAITS_H_
#define ETL_COMMON_TYPE_TRAITS_H_

#include "etl/common/attribute_macros.h"

namespace etl {
namespace common {

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


/*
 * Promoting values to types.
 */

template <typename T, T V>
struct IntegralConstant {
  static constexpr T value = V;
  typedef T value_type;
  typedef IntegralConstant<T, V> type;
  constexpr operator T() { return V; }
};

template <bool V>
struct BoolConstant : public IntegralConstant<bool, V> {};

typedef BoolConstant<true>  TrueType;
typedef BoolConstant<false> FalseType;


template <typename T>
struct TypeConstant {
  typedef T type;
};

/*
 * Fundamental types.
 */

#define ETL_COMMON_SPECIALIZE(tmpl, type, value) \
  template <> \
  struct tmpl<type> : public BoolConstant<value> {}

template <typename T>
struct IsUnqualifiedIntegral : public FalseType {};

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

template <typename T>
struct IsIntegral
  : public IsUnqualifiedIntegral<typename RemoveConstVolatile<T>::type> {};

template <typename T>
struct IsUnqualifiedFloatingPoint : public FalseType {};

ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, float, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, double, true);
ETL_COMMON_SPECIALIZE(IsUnqualifiedFloatingPoint, long double, true);

template <typename T>
struct IsFloatingPoint
  : public IsUnqualifiedFloatingPoint<typename RemoveConstVolatile<T>::type> {};

template <typename T>
struct IsArithmetic
  : public BoolConstant<IsIntegral<T>::value || IsFloatingPoint<T>::value> {};

template <typename T>
struct IsSigned
  : public BoolConstant<IsFloatingPoint<T>::value
                        || (IsIntegral<T>::value && (T(-1) < T(0)))> {};
template <typename T>
struct IsUnsigned
  : public BoolConstant<IsArithmetic<T>::value && !IsSigned<T>::value> {};


/*
 * Reasoning about type relationships.
 */

template <typename T, typename S>
struct IsSame : public FalseType {};

template <typename T>
struct IsSame<T, T> : public TrueType {};


template <bool Selector, typename A, typename B> struct Select;

template <typename A, typename B>
struct Select<true, A, B> {
  typedef A type;
};

template <typename A, typename B>
struct Select<false, A, B> {
  typedef B type;
};

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_TYPE_TRAITS_H_
