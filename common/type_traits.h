#ifndef ETL_COMMON_TYPE_TRAITS_H_
#define ETL_COMMON_TYPE_TRAITS_H_

/*
 * Analog of the C++11 type_traits header.
 */

#include "etl/common/attribute_macros.h"
#include "etl/common/size.h"

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
 * SelectBySize searches a list of types for one with the specified size,
 * as measured by sizeof.
 */
template <Size N, typename... Types> struct SelectBySize;

template <Size N, typename Head, typename... Rest>
struct SelectBySize<N, Head, Rest...>
  : public Conditional<sizeof(Head) == N,
                       TypeConstant<Head>,
                       SelectBySize<N, Rest...>>::type {};


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
 * RemoveVolatile and RemoveConst strip individual qualifiers from a type.
 * RemoveQualifiers does both.
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
struct RemoveQualifiers
  : public RemoveVolatile<typename RemoveConst<T>::type> {};

/*
 * MatchConst and MatchVolatile copy the const/volatile qualifiers from one
 * type onto another.  MatchQualifiers does both.
 */
template <typename Source, typename Dest>
struct MatchConst : public TypeConstant<Dest> {};

template <typename Source, typename Dest>
struct MatchConst<Source const, Dest> : public TypeConstant<Dest const> {};


template <typename Source, typename Dest>
struct MatchVolatile : public TypeConstant<Dest> {};

template <typename Source, typename Dest>
struct MatchVolatile<Source volatile, Dest>
  : public TypeConstant<Dest volatile> {};


template <typename Source, typename Dest>
struct MatchQualifiers
  : public MatchConst<Source, typename MatchVolatile<Source, Dest>::type> {};


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
  : public IsUnqualifiedIntegral<typename RemoveQualifiers<T>::type> {};

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
  : public IsUnqualifiedFloatingPoint<typename RemoveQualifiers<T>::type> {};

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

/*
 * The IsEnum predicate matches enumerations.  Note that this uses a compiler
 * intrinsic; it's difficult to recognize enumerations reliably without this.
 */
template <typename T>
struct IsEnum : public BoolConstant<__is_enum(T)> {};


/*******************************************************************************
 * Deriving types from others or parameters
 */

template <Size N>
struct SignedIntOfSize
  : SelectBySize<N, signed char, short, int, long, long long> {};

template <typename T> struct MakeArithmeticUnsigned;

#define ETL_COMMON_TYPEMAP(tmpl, source, dest) \
  template <> \
  struct tmpl<source> { \
    typedef dest type; \
  }

ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, char,        unsigned char);
ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, signed char, unsigned char);
ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, short,       unsigned short);
ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, int,         unsigned int);
ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, long,        unsigned long);
ETL_COMMON_TYPEMAP(MakeArithmeticUnsigned, long long,   unsigned long long);

template <Size N>
struct UnsignedIntOfSize
  : MakeArithmeticUnsigned<typename SignedIntOfSize<N>::type> {};

template <typename T>
struct MakeUnsigned {
 private:
  template <typename S,
            bool Integral = IsIntegral<S>::value,
            bool Enum = IsEnum<S>::value>
  struct Helper;

  template <typename S>
  struct Helper<S, true, false> {
   private:
    typedef MakeArithmeticUnsigned<typename RemoveQualifiers<S>::type> step1;

   public:
    typedef typename MatchQualifiers<S, typename step1::type>::type type;
  };

  template <typename S>
  struct Helper<S, false, true> {
   private:
    typedef UnsignedIntOfSize<sizeof(S)> step1;
   public:
    typedef typename MatchQualifiers<S, typename step1::type>::type type;
  };

  typedef typename Helper<T>::type type;
};

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_TYPE_TRAITS_H_
