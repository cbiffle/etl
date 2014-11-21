#ifndef _ETL_TYPE_LIST_H_INCLUDED
#define _ETL_TYPE_LIST_H_INCLUDED

/*
 * Support for compile-time lists of types.
 *
 * This is directly inspired by Andrei Alexandrescu's TypeList description
 * from 'Modern C++ Design'.  The actual implementation is simplified to use
 * C++11 variadic templates.
 *
 * Because variadic template parameter packs are not first class, TypeList
 * still has a role in C++11.  While one cannot typedef a parameter pack,
 * one most certainly *can* typedef a TypeList containing it:
 *
 *   template <typename ... Ts>
 *   struct SomeStuff {
 *     using TypeArgs = etl::TypeList<Ts...>;
 *   };
 */

#include <cstddef>

#include "etl/invoke.h"

namespace etl {

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

}  // namespace _type_list

/*
 * A compile-time list of types.
 */
template <typename ... Types>
struct TypeList {
  /*
   * A new TypeList where each type T in the original has been replaced by
   * Invoke<F<T>>.
   */
  template <template <typename> class F>
  using Map = TypeList<Invoke<F<Types>>...>;

  /*
   * The first type T in this TypeList such that Pred<T>::value is true.
   */
  template <template <typename> class Pred>
  using FindFirst = Invoke<_type_list::FindFirstHelper<Pred, Types...>>;

  /*
   * The type in this TypeList with the given index.
   */
  template <std::size_t Index>
  using At = Invoke<_type_list::IndexHelper<Index, Types...>>;

  /*
   * Number of types in this TypeList.
   *
   * This is a constexpr static member function to parallel the structure of
   * etl::IndexSequence (and std::index_sequence), but it's not clear why this
   * is the right thing...
   */
  static constexpr std::size_t size() {
    return sizeof...(Types);
  }
};

}  // namespace etl

#endif  // _ETL_TYPE_LIST_H_INCLUDED
