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
 * Obtaining (fake) values from types.
 */

/*
 * Stands in for operations that produce a value of type T (rvalue reference).
 * This function is never defined, and cannot be actually called.  It's
 * intended for use in unevaluated contexts, like sizeof and decltype.
 *
 * Equivalent to std::declval, which is unfortunately in <utility>.  We define
 * it here, rather than in <utility>, after libstdc++.
 */
template <typename T>
auto declval() -> typename std::add_rvalue_reference<T>::type;


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
 * Template metafunction versions of sizeof, alignof.
 */

template <typename T>
using SizeOf = std::integral_constant<std::size_t, sizeof(T)>;

template <typename T>
using AlignOf = std::integral_constant<std::size_t, alignof(T)>;

/*******************************************************************************
 * Reasoning about type relations.
 */

/*
 * Type alias version of std::result_of.
 */
template <typename F>
using ResultOf = typename std::result_of<F>::type;

/*
 * Type alias version of std::common_type.
 */
template <typename A, typename B>
using CommonType = typename std::common_type<A, B>::type;

}  // namespace etl

#endif  // _ETL_TYPE_TRAITS_H_INCLUDED
