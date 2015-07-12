#ifndef _ETL_NON_NULL_H_INCLUDED
#define _ETL_NON_NULL_H_INCLUDED

/**
 * @defgroup non_null Non-Null Pointer
 *
 * A simple smart pointer that cannot be null, enabling optimizations on null
 * checks.
 *
 * Where `P *` can be `nullptr`, `NonNull<P *>` cannot.
 *
 * This is valuable as documentation, but it also provides checks that cannot
 * accidentally be forgotten or subverted.  `NonNull<P *>` is designed to act
 * as a literal type, and (unlike diligent manual checking for `nullptr`) can
 * actually compile all checks away in cases where the compiler proves a
 * `nullptr` is impossible.
 *
 *
 * Background
 * ----------
 *
 * Null checks in C++ have traditionally been ad-hoc.  Many programs rely on
 * taking a hardware exception if they abuse a null pointer, though in embedded
 * systems this is not always available or guaranteed.  Paranoid programs check
 * for nulls as precondition enforcement, e.g.
 *
 * ~~~{.cc}
 * bool do_the_thing(Foo * foo) {
 *   ASSERT(foo);
 *   return foo->operate();
 * }
 * ~~~
 *
 * This works, but it has problems:
 *
 *  1. It is easy to forget the `ASSERT`.  If the function were refactored or
 *     inlined, the checks might end up in the wrong place.
 *
 *  2. It places the burden on the *callee*, who has the least information about
 *     the pointer.  This means that, without inlining or expensive
 *     whole-program analysis, such checks can't be optimized away.  On the
 *     other hand, the *caller* may be generating that pointer by taking some
 *     object's address, so it can be guaranteed non-null -- the caller has
 *     more knowledge.
 *  3. The signature of `do_the_thing` doesn't indicate to potential callers
 *     that the `Foo *` must be non-null.  At best, the comments will show this.
 *     At worst, callers may need to reference the source.
 *
 *  4. If the requirements for this pointer change during maintenance, the
 *     compiler won't help detect callsites where a nullable pointer -- or even
 *     an explicit nullptr constant -- are being passed.
 * 
 *
 * The NonNull Annotation
 * ----------------------
 *
 * NonNull is a template that acts as a type annotation.  We can rewrite the
 * function above as:
 *
 * ~~~{.cc}
 * bool do_the_thing(NonNull<Foo *> foo) {
 *   return foo->operate();
 * }
 * ~~~
 * 
 * This version addresses each of the four points above:
 *
 *  1. The nullness is now a property of the type, and not an after-effect of
 *     the `ASSERT` -- so we can't forget to check it or lose it on refactoring.
 *
 *  2. The *caller* is now responsible for satisfying the precondition by
 *     providing a non-null pointer -- either using a non-null pointer received
 *     from somewhere else, taking the address of an object, or performing an
 *     explicit null check.
 *
 *  3. The function's signature clearly indicates that the pointer must not be
 *     null.
 *
 *  4. If we were to introduce NonNull on an existing function, any potentially
 *     null callsites would become *immediate* compiler errors.
 *
 *
 * On Costs
 * --------
 *
 * Of course, in the worst case, this has moved the null check from the function
 * to *every caller of the function*.  In practice this is only true on the
 * frontier between explicitly non-null code and code that freely passes null in
 * pointer's clothing.  Like const-correctness, propagating non-nullability
 * outward through APIs makes the checks -- and runtime cost -- go away.
 * 
 * Even for small areas, the checks can be eliminated by propagating
 * non-nullability out to one of the following points:
 * 
 *  - A pointer derived by taking the address of an object, i.e. `&foo`.
 *  - A compile-time constant expression NonNull<P *>.
 *
 * In the embedded systems ETL targets, one of these two points is usually hit
 * quickly -- frequently the second one, when a pointer is loaded from a ROM
 * table.
 *
 * To keep the costs clear, NonNull avoids implicit conversions.  To create a
 * NonNull pointer, you can:
 *
 *  - Use `etl::addrof(P &)`, which is free.
 *  - Use `etl::null_check(P *)`, which [asserts](@ref ETL_ASSERT) if given
 *    `nullptr`.
 *
 * To convert it back to a raw pointer, you must explicitly call the `get()`
 * member function, or use an explicit cast.  This avoids surprise round trips
 * into raw pointer territory and back, which would multiply the runtime checks.
 * 
 * 
 * Implementation Note
 * -------------------
 *
 * NonNull always contains a raw pointer, and cannot contain a smart pointer.
 * This is because common smart pointer types are not literal types, and can
 * silently become null behind the scenes (looking at you, `unique_ptr`).  In
 * general, NonNull is incompatible with smart pointers as they usually appear.
 * In practice, for the embedded applications ETL targets, this is not a
 * hardship -- though if it can be fixed that might be nice.
 *
 * Syntactically, `NonNull<P *>` is spelled thusly, and not `NonNull<P>` or
 * `NonNullPtr<P>`, to emphasize its role as a type annotation.
 * 
 * @{
 */

/** @file
 * Provides etl::NonNull and associated operations.
 */

#include <cstddef>
#include <type_traits>

#include "etl/assert.h"
#include "etl/implicit.h"

namespace etl {

/**
 * A type annotation indicating that the underlying pointer type `P` isn't
 * null.
 *
 * @see non_null
 */
template <typename P>
class NonNull {
public:
  static_assert(std::is_pointer<P>::value,
                "NonNull can only be used with raw pointers.");

 

  /****************************************************************//**
   * @name Construction and conversion
   * @{
   */

  /**
   * Accepts a raw pointer of unknown provenance and attempts to make a
   * non-null pointer of it.
   *
   * Any pointer assignable to P can be used here.
   *
   * This is explicit because it implies a runtime check in the general case.
   * If the syntax of the explicit conversion is unwieldy, have a look at
   * etl::null_check and etl::addrof.
   *
   * Precondition: pointer is not null (asserted).
   */
  template <typename Q>
  explicit constexpr NonNull(Q * ptr) : _ptr{(ETL_ASSERT_CE(ptr), ptr)} {}

  /**
   * Copies a non-null pointer of any assignable type.  No checks are
   * performed.
   */
  template <typename Q>
  constexpr NonNull(NonNull<Q *> const & other) : _ptr{other._ptr} {}

  /**@}*/

  /****************************************************************//**
   * @name Pointer-like operations and pointer access
   * @{
   */

  /**
   * The raw (nullable) pointer type.
   */
  using PointerType = P;

  /**
   * The type of the object being pointed to.
   */
  using ElementType = typename std::remove_pointer<PointerType>::type;

  /**
   * Tests the pointer for nullness in explicit bool context.  Since the pointer
   * is not null, this is pretty simple; it's here for compatibility with the
   * pointer concept in generic code.
   */
  constexpr explicit operator bool() const { return true; }

  /**
   * Dereferences the pointer.
   */
  constexpr ElementType & operator*() const { return *_ptr; }

  /**
   * Member access.
   */
  constexpr PointerType operator->() const { return _ptr; }

  /**
   * Gets the raw pointer, which can then be passed to legacy code, etc.
   */
  constexpr PointerType get() const { return _ptr; }

  /**
   * As syntactic sugar for get(), we also support an explicit conversion.
   */
  explicit constexpr operator PointerType() const { return get(); }


  /**@}*/

  /****************************************************************//**
   * @name Bits for STL compatibility
   * @{
   */

  /**
   * Equivalent of `PointerType` for STL compatibility.
   */
  using pointer = PointerType;

  /**
   * Equivalent of `ElementType` for STL compatibility.
   */
  using element_type = ElementType;


  /**@}*/

  /****************************************************************//**
   * @name Things you can't do
   * @{
   */

  /**
   * Forbids constructing a NonNull from a literal nullptr.
   */
  constexpr NonNull(std::nullptr_t) = delete;

  /**
   * Forbids default construction of a NonNull pointer, since there's no
   * default value possible.
   */
  constexpr NonNull() = delete;

  /**@}*/

private:
  P _ptr;

  template <typename T>
  friend class NonNull;
};

/***************************************************************************//**
 * @name Relations for non-null and normal pointers.
 *
 * These are defined in terms of the underlying pointer types.
 *
 * Note that relational operators with nullptr are not defined, even though they
 * would be trivial.  This provides some assurance against code like this:
 *
 * ~~~{.cc}
 * NonNull<char *> p;
 * // ... time passes ...
 * if (p != nullptr) { ...}
 * ~~~
 *
 * @relates etl::NonNull
 * @{
 */
#define _ETL_NON_NULL_RELATIONAL_OPERATOR(_op_) \
  template <class A, class B> \
  constexpr bool operator _op_(NonNull<A> const & a, NonNull<B> const & b) { \
    return a.get() _op_ b.get(); \
  } \
  \
  template <class A, class B> \
  constexpr bool operator _op_(NonNull<A> const &a, B * b) { \
    return a.get() _op_ b; \
  } \
  \
  template <class A, class B> \
  constexpr bool operator _op_(A * a, NonNull<B> const & b) { \
    return a _op_ b.get(); \
  }

_ETL_NON_NULL_RELATIONAL_OPERATOR(==)
_ETL_NON_NULL_RELATIONAL_OPERATOR(!=)
_ETL_NON_NULL_RELATIONAL_OPERATOR(<)
_ETL_NON_NULL_RELATIONAL_OPERATOR(>)
_ETL_NON_NULL_RELATIONAL_OPERATOR(<=)
_ETL_NON_NULL_RELATIONAL_OPERATOR(>=)

#undef _ETL_NON_NULL_RELATIONAL_OPERATOR
/**@}*/

/***************************************************************************//**
 * @name Address conversion functions.
 * @{
 */

/**
 * Checks a raw pointer for nullness and returns a NonNull version.  This is
 * syntactic sugar for the explicit conversion and involves a runtime check
 * where the value is not provably non-null.
 *
 * @relatesalso etl::NonNull
 */
template <typename P>
constexpr NonNull<P *> null_check(P * ptr) {
  return NonNull<P *>{ ptr };
}

/**
 * Takes the address of an lvalue as a NonNull pointer.  This is equivalent to,
 * but slightly more terse than, @ref null_check "etl::null_check(&foo)".  Note
 * that no runtime check is required, since C++ assumes that references are
 * non-null.
 *
 * @relatesalso etl::NonNull
 */
template <typename P>
constexpr NonNull<P *> addrof(P & obj) {
  return NonNull<P *>{ &obj };
}

/**@}*/

}  // namespace etl

/**@}*/

#endif  // _ETL_NON_NULL_H_INCLUDED
