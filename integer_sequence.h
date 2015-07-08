#ifndef _ETL_INTEGER_SEQUENCE_H_INCLUDED
#define _ETL_INTEGER_SEQUENCE_H_INCLUDED

#include <cstddef>
#include <type_traits>

#include "etl/type_traits.h"

namespace etl {

/*
 * A compile-time sequence of integers.
 *
 * This is a backport of C++14's integer_sequence with the addition of two
 * members for non-empty instantiations:
 *
 * - `head` is the first element.
 * - `Tail` is the IntegerSequence containing elements after the first.
 * - `tail()` yields a value of `Tail` type.
 */
template <typename T, T ... Values>
struct IntegerSequence;

// Empty specialization without head or Tail.
template <typename T>
struct IntegerSequence<T> {
  static constexpr std::size_t size() {
    return 0;
  }
};

// Non-empty specialization.
template <typename T, T v, T ...vs>
struct IntegerSequence<T, v, vs...> {
  static constexpr std::size_t size() {
    return sizeof...(vs) + 1;
  }

  static constexpr T head = v;
  using Tail = IntegerSequence<T, vs...>;
  static constexpr Tail tail() { return {}; }
};


// Helper struct for Concat.
template <typename S1, typename S2>
struct ConcatHelper;

template <typename T, T ... S1, T ... S2>
struct ConcatHelper<IntegerSequence<T, S1...>, IntegerSequence<T, S2...>>
  : TypeConstant<IntegerSequence<T, S1..., S2...>> {};

/*
 * The concatenation of two compile-time integer sequences, which must be of the
 * same type.
 */
template <typename S1, typename S2>
using Concat = typename ConcatHelper<S1, S2>::Type;


// Helper struct for MakeIntegerSequence.
template <typename T, T N, T B = T(0), bool = N == T(0), bool = N == T(1)>
struct MakeIntegerSequenceHelper {
  using Left = typename MakeIntegerSequenceHelper<T, N / 2, B>::Type;
  using Right =
      typename MakeIntegerSequenceHelper<T, N - N / 2, B + N / 2>::Type;

  using Type = Concat<Left, Right>;
};

template <typename T, T N, T B>
struct MakeIntegerSequenceHelper<T, N, B, false, true>
  : TypeConstant<IntegerSequence<T, B>> {};

template <typename T, T N, T B>
struct MakeIntegerSequenceHelper<T, N, B, true, false>
  : TypeConstant<IntegerSequence<T>> {};

/*
 * The compile-time integer sequence of type T from B to N-1.  In the common
 * case, B is 0.
 *
 * This is a backport of C++14's make_integer_sequence.
 */
template <typename T, T N, T B = T(0)>
using MakeIntegerSequence = typename MakeIntegerSequenceHelper<T, N, B>::Type;


/*
 * IndexSequence is merely shorthand for the common case of an IntegerSequence
 * of size_t.
 *
 * This is a backport of C++14's index_sequence.
 */
template <std::size_t ... Values>
using IndexSequence = IntegerSequence<std::size_t, Values...>;

/*
 * Version of MakeIntegerSequence adapted to IndexSequence.
 *
 * This is a backport of C++14's make_index_sequence.
 */
template <std::size_t N, std::size_t B = 0>
using MakeIndexSequence = MakeIntegerSequence<std::size_t, N, B>;

/*
 * The index sequence that parallels the given list of types -- that is, for
 * a list of N types, the sequence 0 .. N-1.  This is useful when e.g.
 * forwarding function arguments from a tuple.
 *
 * This is a backport of C++14's index_sequence_for.
 */
template <typename ... T>
using IndexSequenceFor = MakeIndexSequence<sizeof...(T)>;

}  // namespace etl

#endif  // _ETL_INTEGER_SEQUENCE_H_INCLUDED
