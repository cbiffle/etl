#ifndef _ETL_INVOKE_H_INCLUDED
#define _ETL_INVOKE_H_INCLUDED

#include "etl/type_constant.h"

namespace etl {

/*
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

}  // namespace etl

#endif  // _ETL_INVOKE_H_INCLUDED
