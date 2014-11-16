#ifndef _ETL_TYPE_TRAITS_H_INCLUDED
#define _ETL_TYPE_TRAITS_H_INCLUDED

/*
 * Trait templates describing properties of types and providing basic
 * operations over types.
 *
 * Provides extensions to C++ <type_traits>.
 */

#include <cstddef>
#include <climits>
#include <type_traits>

#include "etl/attribute_macros.h"

namespace etl {

/*******************************************************************************
 * Promoting values to types
 */

// Shorthand for the common case of std::integral_constant<bool, x>
template <bool V>
using BoolConstant = std::integral_constant<bool, V>;


/*******************************************************************************
 * Promoting types to values.
 */

/*
 * TypeConstant creates a type of trivial values associated with a given type.
 * This can be used to pass a placeholder for a type in a value context,
 * without any overhead that might be associated with creating an actual
 * instance of the type (or even a pointer).
 *
 * For compatibility, this exposes both std-style and etl-style names.
 */
template <typename T>
struct TypeConstant {
  using Type = T;
  using type = T;
};


/*******************************************************************************
 * Converting between C++03-style member type metafunctions and C++11-style
 * alias metafunctions.
 *
 * In general, the syntax "typename T::type" or "typename T::Type" can be
 * replaced by "etl::Invoke<T>", regardless of the complexity of T.
 *
 * If both 'type' and 'Type' members are present, the uppercase one will be
 * preferred.
 */

// Helper class for distinguishing std from etl conventions
template <typename T>
struct HasUppercaseTypeMember {
private:
  /*
   * Idiom alert.
   *
   * We want to determine if T has a member type named Type.  If we try to
   * reference it and it doesn't exist, we'll take a compiler error.
   *
   * But if we try to reference it in a template context and it's *ambiguous*,
   * we get a pass.
   *
   * So, we introduce ambiguity by extending T and adding a Type member type.
   *
   * If T had a Type member already, the "No" overload of test below -- normally
   * treated as more specific and a better overload match -- will be ambiguous
   * and the compiler will disable it.
   *
   * If T has no Type member, we've added one in Derived, so the "No" overload
   * will be unambiguously preferred.
   *
   * Here's hoping that some future version of C++ adds some kind of "syntactic
   * validity predicates" that can test this more cleanly, such as the
   * 'requires' blocks proposed in Concepts Lite.
   */
  using Yes = char[2];
  using No = char[1];

  struct Fallback { struct Type {}; };
  struct Derived : T, Fallback {};

  template <class U>
  static No & test(typename U::Type *);

  template <class U>
  static Yes & test(U *);

public:
  static constexpr bool value = sizeof(test<Derived>(nullptr)) == sizeof(Yes);
};

template <typename T, bool = HasUppercaseTypeMember<T>::value>
struct InvokeHelper : public TypeConstant<typename T::type> {};

template <typename T>
struct InvokeHelper<T, true> : public TypeConstant<typename T::Type> {};

template <typename T>
using Invoke = typename InvokeHelper<T>::Type;


/*******************************************************************************
 * Compile-time conditional behavior.
 */

/*
 * ConditionalT is a backport of C++14's conditional_t.
 */
template <bool B, typename T, typename F>
using ConditionalT = Invoke<std::conditional<B, T, F>>;


/*******************************************************************************
 * Operations on qualifiers.
 */

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
using MatchQualifiers =
    MatchConst<Source, typename MatchVolatile<Source, Dest>::Type>;


/*******************************************************************************
 * Reasoning about type sizes.
 */

/*
 * SelectBySize searches a list of types for one with the specified size,
 * as measured by sizeof.
 */
template <std::size_t N, typename... Types> struct SelectBySize;

// Common case: linear recursion.
template <std::size_t N, typename Head, typename... Rest>
struct SelectBySize<N, Head, Rest...>
  : public std::conditional<sizeof(Head) == N,
                            TypeConstant<Head>,
                            SelectBySize<N, Rest...>>::Type {};

// Termination case for single type, to improve error reporting.
template <std::size_t N, typename OnlyCandidate>
struct SelectBySize<N, OnlyCandidate> {
  static_assert(sizeof(OnlyCandidate) == N,
                "No type in SelectBySize list had the required size!");
  typedef OnlyCandidate Type;
};

static constexpr std::size_t char_bits = CHAR_BIT;


/*******************************************************************************
 * Selecting integer types by size (as measured by sizeof).
 */

template <std::size_t N>
using SignedIntOfSize =
    SelectBySize<N, signed char, short, int, long, long long>;

template <std::size_t N>
struct UnsignedIntOfSize {
  typedef typename std::make_unsigned<typename SignedIntOfSize<N>::Type>::type
          Type;
};

}  // namespace etl

#endif  // _ETL_TYPE_TRAITS_H_INCLUDED
