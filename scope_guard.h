#ifndef _ETL_SCOPE_GUARD_H_INCLUDED
#define _ETL_SCOPE_GUARD_H_INCLUDED

/**
 * @defgroup scope_guard Scope Guard
 *
 * Scoped actions, an alternative to custom RAII types.
 *
 * This is patterned after an idea by Andrei Alexandrescu.
 *
 * Use Case and Examples
 * =====================
 *
 * In C++, the "right" way of ensuring that some cleanup action gets taken when
 * control flow leaves a block is with an RAII object, which performs the
 * cleanup during its destructor.  However, actually *doing* this can be very
 * verbose, so many people don't.
 *
 * ScopeGuard removes the boilerplate to make doing things the "right" way
 * easier than doing them the wrong way.
 *
 * For example, imagine we want to set a GPIO on entry to a function, and reset
 * it on exit.
 *
 * ~~~{.cc}
 * int the_function() {
 *   gpio.set();
 *
 *   if (condition1) {
 *     gpio.reset();
 *     return 0;
 *   }
 *
 *   if (condition2) {
 *     gpio.reset();
 *     return 1;
 *   }
 *     
 *   ...much code...
 *
 *   gpio.reset();
 *   return 42;
 * }
 * ~~~
 *
 * At any potential exit from the function, we have to reset the GPIO.  This
 * is troublesome:
 *
 * - It may be hard for a reader or reviewer to convince themselves that it's
 *   done correctly.
 *
 * - As a result, errors may creep in.
 *
 * - Refactoring the function to have a single return point may cause it to be
 *   easier to read for these purposes, but harder to read for its original
 *   intent -- and doing so may be impossible if there are early exits due to
 *   exceptions or @ref ETL_CHECK.
 *
 * The traditional C++ fix for this is to write a specialized class, perhaps
 * called `ScopedGpio`, that has side effects when it's constructed (set) and
 * destroyed (reset).
 *
 * ~~~{.cc}
 * class ScopedGpio {
 * public:
 *   ScopedGpio(Gpio & gpio) : _gpio(gpio) {
 *     gpio.set();
 *   }
 *   ~ScopedGpio() {
 *     gpio.reset();
 *   }
 * };
 *
 * int the_function() {
 *   ScopedGpio scoped_gpio(gpio);
 *
 *   ... rest of body ...
 *
 *   // GPIO gets reliably reset by scoped_gpio's destructor.
 * }
 * ~~~
 *
 * If the scoping class is reusable, great -- but if it's a one-off, then that's
 * an awful lot of typing to expect from a beleaguered programmer!
 *
 * ETL's scope guard offers a concise alternative in the @ref ETL_ON_SCOPE_EXIT
 * macro:
 *
 * ~~~{.cc}
 * int the_function() {
 *   gpio.set();
 *   ETL_ON_SCOPE_EXIT { gpio.reset(); };
 *
 *   ... rest of body ...
 * }
 * ~~~
 *
 * Any exit path from the scope *after* the use of @ref ETL_ON_SCOPE_EXIT
 * will run the associated code.
 *
 *
 * Dismissing Guards
 * =================
 *
 * But what if there is a path where we *don't* want the guard to do its job?
 * Maybe some other error handling already did it for us.  For such cases, we
 * can *dismiss* the guard.  To do this, we have to give it a name, using @ref
 * etl::make_guard :
 *
 * ~~~{.cc}
 * int the_function() {
 *   gpio.set();
 *   auto guard = etl::make_guard([] { gpio.reset(); });
 *
 *   if (unusual_condition) {
 *     guard.dismiss();  // don't change the GPIO's state from here on.
 *   }
 *
 *   ... rest of body ...
 * }
 * ~~~
 *
 * After calling @ref etl::ScopeGuard::dismiss() "dismiss", the guard is
 * disabled along every exit path from the scope, and the associated code won't
 * be run.
 *
 * In general, @ref ETL_ON_SCOPE_EXIT is more concise, but @ref etl::make_guard
 * is more flexible.
 *
 *
 * Compilation
 * ===========
 *
 * The @ref ETL_ON_SCOPE_EXIT macro generates code very similar to the
 * hand-rolled C-style equivalent using `goto`.
 *
 * etl::make_guard() performs similarly unless you pass it an elaborate function
 * object with virtual methods or something.  The easiest way to do this is
 * by using `std::function`; expect overhead if you do this.
 *
 * @{
 */

/** @file
 * Provides #ETL_ON_SCOPE_EXIT and #etl::make_guard.
 */

#include <type_traits>

#include "etl/concatenate.h"
#include "etl/implicit.h"
#include "etl/utility.h"

namespace etl {

/**
 * Holds a function and calls it at destruction.  This is the underlying type
 * returned by @ref make_guard and used inside @ref ETL_ON_SCOPE_EXIT.
 *
 * ScopeGuard is parameterized by the function type so that it can store the
 * full variety of C++ function-like objects:
 *
 * - Traditional C-like functions.
 * - Static member functions.
 * - Function objects, including bound member functions.
 * - Lambda expressions, whose type is impossible to write by hand.
 *
 * That last case is why you should rarely interact with ScopeGuard directly,
 * either by using the constructors or by writing out its type.  Guards should
 * be created by @ref make_guard to benefit from inference, and written as type
 * `auto`.
 */
template <typename F>
class ScopeGuard {
public:

  /**
   * The underlying function type this guard will call.  This is often a lambda
   * type that may not be directly expressible.  To avoid needing to write the
   * type out, use @ref make_guard, which will infer the proper type.
   */
  using FunctionType = F;

  /** @name Guard lifecycle
   * @{ */

  /**
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

  /**
   * Copying a ScopeGuard would imply that the cleanup action should be
   * performed twice, which doesn't make sense.
   */
  ScopeGuard(ScopeGuard<F> const &) = delete;

  /**
   * Renders this guard inactive.  It will no longer call its function at
   * destruction.
   */
  void dismiss() {
    _active = false;
  }

  /**
   * Destroys the guard, calling the function if the guard is active.
   */
  ~ScopeGuard() {
    if (_active) _fn();
  }

  /**@}*/

private:
  FunctionType _fn;
  bool _active;
};


/**
 * Factory function for ScopeGuards.  The full type of a function object, and
 * thus a ScopeGuard, can be *very* difficult to write correctly -- think of the
 * distinctions between member and normal function pointers, to say nothing of
 * lambda expressions with captures.  This function saves you the trouble by
 * deducing the function type from its argument.  Together with `auto`, you
 * needn't worry about the function's explicit type.
 *
 * @ref make_guard can be used with lambda functions:
 *
 * ~~~{.cc}
 * // Lambda
 * auto guard = etl::make_guard([]{ stuff; });
 * ~~~
 *
 * It can also be used with C-style namespace-scoped functions that take no
 * arguments:
 *
 * ~~~{.cc}
 * // Namespace-scoped function
 * auto guard = etl::make_guard(::abort);
 * ~~~
 *
 * Or static member functions:
 *
 * ~~~{.cc}
 * // Static member function
 * auto guard = etl::make_guard(MyClass::a_function);
 * ~~~
 *
 * Or functor objects:
 *
 * ~~~{.cc}
 * // Functor object
 * auto guard = etl::make_guard(MyFunctor{});
 * ~~~
 *
 * Anything *callable* with no arguments can be used.
 */
template <typename F>
ScopeGuard<typename std::decay<F>::type> make_guard(F && fn) {
  return ScopeGuard<typename std::decay<F>::type> { etl::forward<F>(fn) };
}


/**
 * Syntactic sugar for ScopeGuard for the common case of calling an inline
 * lambda function.
 *
 * These two versions are equivalent:
 *
 * ~~~{.cc}
 * // Explicit guard creation
 * auto guard = etl::make_guard([]{ cleanup_action(); });
 *
 * // Implicit guard through macro
 * ETL_ON_SCOPE_EXIT { cleanup_action(); };
 * ~~~
 *
 * @hideinitializer
 * @internal
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
#define ETL_ON_SCOPE_EXIT \
  auto ETL_CONCATENATE(on_scope_exit, __COUNTER__) = ::etl::GuardTag() << [&]()

// Arbitrary type serving as LHS for the operator below.
struct GuardTag {};

// Operator used to get correct infix behavior for ETL_ON_SCOPE_EXIT.
template <typename F>
ScopeGuard<typename std::decay<F>::type> operator<<(GuardTag const &,
                                                    F && fn) {
  return etl::make_guard(etl::forward<F>(fn));
}

}  // namespace etl

/**@}*/

#endif  // _ETL_SCOPE_GUARD_H_INCLUDED
