#ifndef _ETL_SCOPE_GUARD_H_INCLUDED
#define _ETL_SCOPE_GUARD_H_INCLUDED

#include <type_traits>

#include "etl/concatenate.h"
#include "etl/implicit.h"
#include "etl/utility.h"

namespace etl {

/*
 * Holds a function and calls it at destruction.  This is a utility class that
 * produces RAII classes from lambdas, function pointers, and the like.
 *
 * While specialized RAII classes are often more semantically meaningful and
 * harder to get wrong, ScopeGuard can be very valuable for either one-off cases
 * or bridging between legacy libraries and C++ code.
 *
 * For example, imagine we want to track how many (recursive) calls into a
 * particular function are currently on the stack.
 *
 *   static unsigned recursion_count = 0;
 *
 *   int the_function() {
 *     ++recursion_count;
 *
 *     if (condition1) {
 *       --recursion_count;
 *       return 0;
 *     }
 *
 *     if (condition2) {
 *       --recursion_count;
 *       return 1;
 *     }
 *     
 *     ...
 *     --recursion_count;
 *     return 42;
 *   }
 *
 * Note that, at any potential exit from the function, we have to update
 * recursion_count.  (While the example above could probably be simplified using
 * a single return statement, code similar to the example can result implicitly
 * from uses of ETL_CHECK.)
 *
 * The traditional C++ fix for this is to write a specialized RAII class,
 * perhaps called IncrementCount, that has side effects when it's constructed
 * (increment) and destroyed (decrement).  But this is verbose and turns many
 * programmers off from fully embracing RAII.
 *
 * ScopeGuard offers a concise alternative:
 *
 *   int the_function() {
 *     ++recursion_count;
 *     auto guard = etl::make_guard([] { --recursion_count; });
 *
 *     ... rest of body ...
 *   }
 *
 * This ensures that on *any* exit path from the scope, the lambda will be
 * invoked.  Effectively, we have used a template to roll a new RAII class from
 * an inline expression, without the traditional declaration.
 *
 * But what if there is a path where we *don't* want the guard to do its job?
 * For such cases, we can dismiss the guard:
 *
 *   int the_function() {
 *     ++recursion_count;
 *     auto guard = etl::make_guard([] { --recursion_count; });
 *
 *     if (unusual_condition) {
 *       guard.dismiss();  // leave recursion_count unchanged
 *     }
 *
 *     ... rest of body ...
 *   }
 *
 * Note that all the examples above use the `etl::make_guard` function to
 * produce guards.  This is what you want to do in the vast majority of cases;
 * see the discussion on `make_guard` below.
 *
 * You may also be interested in the ETL_ON_SCOPE_EXIT macro defined at the end
 * of this file.
 *
 *
 * Implementation Note
 * -------------------
 *
 * ScopeGuard is parameterized by the function type so that it can store the
 * full variety of C++ function-like objects:
 *
 * - Traditional C-like functions.
 * - Static member functions.
 * - Function objects, including bound member functions.
 * - Lambda expressions, whose type is impossible to write by hand.
 */
template <typename F>
class ScopeGuard {
public:
  /*
   * The underlying function type this guard will call.
   */
  using FunctionType = F;

  /*
   * Creates a new ScopeGuard from a function.  The guard is initially active
   * and will call the function at destruction.
   */
  explicit ScopeGuard(FunctionType && fn)
    : _fn{etl::forward<FunctionType>(fn)},
      _active{true} {}

  /*
   * Moves a ScopeGuard.  The original guard is dismissed.  The resulting guard
   * will be active iff the original was active before the move.
   */
  ScopeGuard(ScopeGuard<FunctionType> && other)
    : _fn{etl::move(other._fn)},
      _active{etl::move(other._active)} {
    other._active = false;
  }

  /*
   * Copying a ScopeGuard would imply that the cleanup action should be
   * performed twice, which doesn't make sense.
   */
  ScopeGuard(ScopeGuard<F> const &) = delete;

  /*
   * Renders this guard inactive.  It will no longer call its function at
   * destruction.
   */
  void dismiss() {
    _active = false;
  }

  /*
   * Destroys the guard, calling the function if the guard is active.
   */
  ~ScopeGuard() {
    if (_active) _fn();
  }

private:
  FunctionType _fn;
  bool _active;
};


/*
 * Factory function for ScopeGuards.  The full type of a function object, and
 * thus a ScopeGuard, can be *very* difficult to write correctly -- think of the
 * distinctions between member and normal function pointers, to say nothing of
 * lambda expressions with captures.  This function saves you the trouble by
 * deducing the function type from its argument.  Together with `auto`, you
 * needn't worry about the function's explicit type:
 *
 *   auto guard = etl::make_guard(my_function);
 */
template <typename F>
ScopeGuard<typename std::decay<F>::type> make_guard(F && fn) {
  return ScopeGuard<typename std::decay<F>::type> { etl::forward<F>(fn) };
}


/*******************************************************************************
 * Syntactic sugar for ScopeGuard.  This eliminates the need for a named
 * variable and loses the lambda header and right-hand paren, which can be
 * visually distracting.  In exchange, you lose the ability to dismiss the guard
 * (because you cannot name it), and you must use a lambda rather than a
 * function reference.
 *
 * Where an unsugared version would read:
 *
 *  in_routine = true;
 *  auto guard = etl::make_guard([] { in_routine = false; });
 *
 * The sugared version reads:
 *
 *  in_routine = true;
 *  ETL_ON_SCOPE_EXIT { in_routine = false; };
 *
 * Note the trailing semicolon -- it is mandatory.
 *
 *
 * Implementation Note
 * -------------------
 * 
 * To achieve the syntax above using only textual substitution (i.e. the C
 * preprocessor), the definition of ETL_ON_SCOPE_EXIT must not open a function
 * argument list.  (Otherwise we'd need a weird right-hand paren.)
 *
 * One *could* try to define a prefix operator for function types and implement
 * the macro in terms of this, but that seems quite fragile.
 *
 * Instead, we use an infix operator where the left-hand operand is a simple tag
 * type, to distinguish our operator from possible other overloads.
 *
 * We arbitrarily choose the "gozinta" operator, <<.  Note that, unlike chevron
 * I/O, our operator cannot be chained -- it's really intended for use by the
 * macro only.
 */

struct GuardTag {};

template <typename F>
ScopeGuard<typename std::decay<F>::type> operator<<(GuardTag const &,
                                                    F && fn) {
  return etl::make_guard(etl::forward<F>(fn));
}

#define ETL_ON_SCOPE_EXIT \
  auto ETL_CONCATENATE(on_scope_exit, __COUNTER__) = ::etl::GuardTag() << [&]()

}  // namespace etl

#endif  // _ETL_SCOPE_GUARD_H_INCLUDED
