#ifndef _ETL_TYPE_LIST_H_INCLUDED
#define _ETL_TYPE_LIST_H_INCLUDED

/**
 * @defgroup type_list TypeList
 *
 * A representation for lists of types, and utilities for working with them.
 *
 * This is directly inspired by Andrei Alexandrescu's %TypeList description
 * from 'Modern C++ Design'.  The actual implementation is simplified to use
 * C++11 variadic templates.
 *
 * Rationale for TypeList in C++11
 * ===============================
 *
 * Alexandrescu's implementation was written in terms of C++03.  Many of the
 * use cases for it, like creating a generic tuple class, were obviated by the
 * introduction of variadic templates in C++11.  As a result, it's tempting to
 * assume that we no longer have any need for TypeLists in this brave new
 * world.
 *
 * Not so!  Variadic template parameter packs, while very flexible, are not
 * *first class* in the type system.  For example, a template instantiated
 * with a parameter pack cannot *re-export* its types easily:
 *
 * ~~~{.cc}
 * template <typename ... Ts>
 * struct SomeStuff {
 *   using TypeArgs = Ts...;  // Can't do this.
 * };
 * ~~~
 *
 * We need a type that stands in for the parameter pack.  That type is TypeList.
 *
 * ~~~{.cc}
 * template <typename ... Ts>
 * struct SomeStuff {
 *   using TypeArgs = TypeList<Ts...>;  // Problem solved.
 * };
 * ~~~
 *
 * Another case: class templates can only have one parameter pack in their
 * template parameter list:
 *
 * ~~~{.cc}
 * // Can't do this
 * template <typename ... Xs, typename ... Ys>
 * struct Processor {
 *   void process_xs(Xs...);
 *   void process_ys(Ys...);
 * };
 * ~~~
 *
 * TypeList can wrap several parameter packs and keep them neatly separated,
 * at the cost of a specialization step:
 *
 * ~~~{.cc}
 * template <typename XList, typename YList>
 * struct Processor;
 *
 * template <typename ... Xs, typename ... Ys>
 * struct Processor<TypeList<Xs...>, TypeList<Ys...>> {
 *   void process_xs(Xs...);
 *   void process_ys(Ys...);
 * };
 * ~~~
 *
 * Finally, because we can now refer to a list of types, we can define
 * operations over them.
 *
 * ~~~{.cc}
 * TypeList<char, ScopeGuard, int *>::contains<int *>()  // => true
 * ~~~
 *
 * @{
 */

/** @file
 *
 * Provides TypeList and friends.
 */

#include <cstddef>
#include <type_traits>

#include "etl/invoke.h"
#include "etl/type_traits.h"

namespace etl {

/*******************************************************************************
 * Internal implementation details
 */

template <typename ... Types>
struct TypeList;

namespace _type_list {

template <
  bool comparison_result,
  typename First,
  template <typename> class Pred,
  typename ... Rest>
struct FindFirstHelper_ {
  static_assert(sizeof...(Rest) != 0,
                "TypeList::FindFirst found no matching types");
};

template <
  typename Bad,
  template <typename> class Pred,
  typename NewHead,
  typename ... Rest>
struct FindFirstHelper_<false, Bad, Pred, NewHead, Rest...>
  : FindFirstHelper_<Pred<NewHead>::value, NewHead, Pred, Rest...> {};

template <
  typename Result,
  template <typename> class Pred,
  typename ... Rest>
struct FindFirstHelper_<true, Result, Pred, Rest...> {
  using Type = Result;
};


template <template <typename> class Pred, typename ... Types>
struct FindFirstHelper {
  static_assert(sizeof...(Types) != 0,
                "TypeList::FindFirst is not valid for an empty TypeList");
};

template <template <typename> class Pred, typename First, typename ... Rest>
struct FindFirstHelper<Pred, First, Rest...>
  : FindFirstHelper_<Pred<First>::value, First, Pred, Rest...> {};


template <std::size_t N, typename ... Types>
struct IndexHelper {
  static_assert(sizeof...(Types) != 0,
                "TypeList index out of range");
};

template <typename Head, typename ... Rest>
struct IndexHelper<0, Head, Rest...> {
  using Type = Head;
};

template <std::size_t N, typename Head, typename ... Rest>
struct IndexHelper<N, Head, Rest...> : IndexHelper<N - 1, Rest...> {};

template <typename, typename ...>
struct ContainsHelper : std::integral_constant<bool, false> {};

template <typename Target, typename First, typename ... Rest>
struct ContainsHelper<Target, First, Rest...>
  : std::integral_constant<bool, std::is_same<Target, First>::value ? true
                      : ContainsHelper<Target, Rest...>::value> {};

template <template<typename>class, typename ...>
struct AnyHelper : std::integral_constant<bool, false> {};

template <template<typename> class P, typename First, typename ... Rest>
struct AnyHelper<P, First, Rest...>
  : std::integral_constant<bool, P<First>::value ? true
                      : AnyHelper<P, Rest...>::value> {};

template <template<typename>class, typename ...>
struct AllHelper : std::integral_constant<bool, true> {};

template <template<typename> class P, typename First, typename ... Rest>
struct AllHelper<P, First, Rest...>
  : std::integral_constant<bool, !P<First>::value ? false
                      : AllHelper<P, Rest...>::value> {};

template <typename ... Types>
struct UniqueHelper : std::integral_constant<bool, true> {};

template <typename First, typename ... Types>
struct UniqueHelper<First, Types...> : std::integral_constant<bool,
  !ContainsHelper<First, Types...>::value && UniqueHelper<Types...>::value
> {};

template <typename T, std::size_t count, typename... R>
struct RepeatHelper {
  using Type = typename RepeatHelper<T, count - 1, T, R...>::Type;
};

template <typename T, typename... R>
struct RepeatHelper<T, 0, R...> {
  using Type = TypeList<R...>;
};

template <template <typename> class F, typename ...>
struct MaxF;

template <template <typename> class F, typename First, typename ... Rest>
struct MaxF<F, First, Rest...>
  : std::integral_constant<decltype(F<First>::value),
                           (F<First>::value > MaxF<F, Rest...>::value)
                              ? F<First>::value
                              : MaxF<F, Rest...>::value> {};

template <template <typename> class F, typename First>
struct MaxF<F, First>
  : std::integral_constant<decltype(F<First>::value),
                           F<First>::value> {};

template <typename First, typename ... Rest>
using MaxSizeOf = MaxF<etl::SizeOf, First, Rest...>;

template <typename First, typename ... Rest>
using MaxAlignOf = MaxF<etl::AlignOf, First, Rest...>;

template <typename T, typename ... Types>
struct IndexOfHelper : std::integral_constant<int, -1> {};

template <typename T, typename First, typename ... Rest>
struct IndexOfHelper<T, First, Rest...>
  : std::integral_constant<int,
                           std::is_same<T, First>::value
                             ? 0
                             : 1 + IndexOfHelper<T, Rest...>::value> {};

}  // namespace _type_list


/*******************************************************************************
 * TypeList itself
 */

/**
 * A compile-time list of types.
 *
 * @see type_list
 */
template <typename ... Types>
struct TypeList {
  /**
   * A new TypeList where each type `T` in the original has been replaced by
   * @ref etl::Invoke "Invoke<F<T>>".
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   * using Pointers = Numbers::Map<std::add_pointer>;
   *   // => TypeList<int *, long *, double *>
   * ~~~
   */
  template <template <typename> class F>
  using Map = TypeList<Invoke<F<Types>>...>;

  /**
   * The first type T in this TypeList such that Pred<T>::value is true.
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   * using SomeFloat = Numbers::FindFirst<std::is_floating_point>;
   *   // => double
   * using SomeInt = Numbers::FindFirst<std::is_integral>;
   *   // => int
   * using SomePtr = Numbers::FindFirst<std::is_pointer>;
   *   // => Compile error: no such type
   * ~~~
   */
  template <template <typename> class Pred>
  using FindFirst = Invoke<_type_list::FindFirstHelper<Pred, Types...>>;

  /**
   * The type in this TypeList with the given index.
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   *
   * using Type0 = Numbers::At<0>;
   *   // => int
   * using Type4 = Numbers::At<4>;
   *   // => Compile error: index out of range
   * ~~~
   */
  template <std::size_t Index>
  using At = Invoke<_type_list::IndexHelper<Index, Types...>>;

  /**
   * Does this TypeList contain the given type?
   *
   * This operation is also available as a value-template, @ref Contains.
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   *
   * Numbers::contains<int>()  // => true
   * Numbers::contains<void>()  // => false
   * ~~~
   */
  template <typename X>
  static constexpr bool contains() {
    return _type_list::ContainsHelper<X, Types...>::value;
  }

  /**
   * Does this TypeList contain the given type?
   *
   * This operation is also available as a constexpr function, @ref contains().
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   *
   * Numbers::Contains<int>::value  // => true
   * Numbers::Contains<void>::value  // => false
   * ~~~
   */
  template <typename X>
  struct Contains
    : std::integral_constant<bool,
                             _type_list::ContainsHelper<X, Types...>::value> {};

  /**
   * Is each type in this TypeList unique?
   *
   * ~~~{.cc}
   * TypeList<>::all_unique  // => true
   * TypeList<int>::all_unique  // => true
   * TypeList<int, int>::all_unique  // => false
   * TypeList<int, char>::all_unique  // => true
   * ~~~
   */
  static constexpr bool all_unique = _type_list::UniqueHelper<Types...>::value;

  /**
   * Number of types in this TypeList.
   *
   * This is a constexpr static member function to parallel the structure of
   * etl::IndexSequence (and std::index_sequence), but it's not clear why this
   * is the right thing...
   */
  static constexpr std::size_t size() {
    return sizeof...(Types);
  }

  /**
   * Index of the given type in this TypeList.
   *
   * ~~~{.cc}
   * using Numbers = TypeList<int, long, double>;
   *
   * Numbers::index_of<int>()     // => 0
   * Numbers::index_of<double>()  // => 2
   * Numbers::index_of<char *>()  // => Compile error: not found
   * ~~~
   */
  template <typename T>
  static constexpr std::size_t index_of() {
    using R = _type_list::IndexOfHelper<T, Types...>;
    static_assert(R::value != -1, "Type not found in type list");

    return std::size_t(R::value);
  }
};


/*******************************************************************************
 * TypeList-associated utilities.
 */

/**
 * `Repeat<T, x>` is a TypeList containing `T` repeated `x` times.
 *
 * ~~~{.cc}
 * Repeat<int, 5>  // => TypeList<int, int, int, int, int>
 * ~~~
 */
template <typename T, std::size_t count>
using Repeat = typename _type_list::RepeatHelper<T, count>::Type;

/**
 * Do any types in the TypeList match predicate template P?  If the TypeList
 * is empty, the result is false.
 */
template <template <typename> class P, typename T>
struct Any;

template <template <typename> class P, typename... Types>
struct Any<P, TypeList<Types...>> : _type_list::AnyHelper<P, Types...> {};

/**
 * Do *none* of the types in the TypeList match predicate template P?  Inverse
 * of Any.
 */
template <template <typename> class P, typename T>
struct None : std::integral_constant<bool, !Any<P, T>::value> {};

/**
 * Do *all* of the types in the TypeList match predicate template P?  If the
 * TypeList is empty, the result is true.
 */
template <template <typename> class P, typename T>
struct All;

template <template <typename> class P, typename... Types>
struct All<P, TypeList<Types...>> : _type_list::AllHelper<P, Types...> {};

/**
 * Maximum size (as measured by `sizeof`) of all the types in the TypeList.
 */
template <typename TL>
struct MaxSizeOf;

template <typename First, typename ... Rest>
struct MaxSizeOf<TypeList<First, Rest...>>
    : _type_list::MaxSizeOf<First, Rest...> {};

/**
 * Maximum alignment (as measured by `alignof`) of all the types in the
 * TypeList.
 */
template <typename TL>
struct MaxAlignOf;

template <typename First, typename ... Rest>
struct MaxAlignOf<TypeList<First, Rest...>>
    : _type_list::MaxAlignOf<First, Rest...> {};

}  // namespace etl

/**@}*/

#endif  // _ETL_TYPE_LIST_H_INCLUDED
