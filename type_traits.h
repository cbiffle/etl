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
 */
template <typename T>
struct TypeConstant {
  typedef T Type;
};


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
