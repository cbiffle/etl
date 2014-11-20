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
#include "etl/invoke.h"
#include "etl/type_constant.h"

namespace etl {

/*******************************************************************************
 * Promoting values to types
 */

// Shorthand for the common case of std::integral_constant<bool, x>
template <bool V>
using BoolConstant = std::integral_constant<bool, V>;


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
 * as measured by sizeof.  If given two types with equal size, the first is
 * preferred.
 */

template <std::size_t N, typename... Types> struct SelectBySizeHelper;

// Common case: linear recursion.
template <std::size_t N, typename Head, typename... Rest>
struct SelectBySizeHelper<N, Head, Rest...>
  : std::conditional<sizeof(Head) == N,
                     TypeConstant<Head>,
                     SelectBySizeHelper<N, Rest...>>::type {};

// Termination case for single type, to improve error reporting.
template <std::size_t N, typename OnlyCandidate>
struct SelectBySizeHelper<N, OnlyCandidate> {
  static_assert(sizeof(OnlyCandidate) == N,
                "No type in SelectBySize list had the required size!");
  typedef OnlyCandidate Type;
};

template <std::size_t N, typename... Types>
using SelectBySize = Invoke<SelectBySizeHelper<N, Types...>>;

/*
 * SelectByMinSize searches a list of types for the first one with at least the
 * specified size, as measured by sizeof.
 */

template <std::size_t N, typename... Types> struct SelectByMinSizeHelper;

// Common case: linear recursion.
template <std::size_t N, typename Head, typename... Rest>
struct SelectByMinSizeHelper<N, Head, Rest...>
  : std::conditional<sizeof(Head) >= N,
                     TypeConstant<Head>,
                     SelectByMinSizeHelper<N, Rest...>>::type {};

// Termination case for single type, to improve error reporting.
template <std::size_t N, typename OnlyCandidate>
struct SelectByMinSizeHelper<N, OnlyCandidate> {
  static_assert(sizeof(OnlyCandidate) >= N,
                "No type in SelectByMinSize list had the required size!");
  typedef OnlyCandidate Type;
};

template <std::size_t N, typename... Types>
using SelectByMinSize = Invoke<SelectByMinSizeHelper<N, Types...>>;

static constexpr std::size_t char_bits = CHAR_BIT;


/*******************************************************************************
 * Selecting integer types by size (as measured by sizeof).
 */

template <std::size_t N>
using SignedIntOfSize =
    SelectBySize<N, signed char, short, int, long, long long>;

template <std::size_t N>
using UnsignedIntOfSize = Invoke<std::make_unsigned<SignedIntOfSize<N>>>;

}  // namespace etl

#endif  // _ETL_TYPE_TRAITS_H_INCLUDED
