Scope Guard
===========

.. contents:: :local:

.. namespace:: etl

*Scope Guard* makes it easier to do something on exit from a scope, such as a
function, even in the presence of exceptions.  It can save you from
hand-writing "RAII" classes.

ETL's implementation of Scope Guard is modeled after Andrei Alexandrescu's.

Use Case and Examples
---------------------

In C++, the "right" way of ensuring that some cleanup action gets taken when
control flow leaves a block is with an RAII object, which performs the cleanup
during its destructor.  However, actually *doing* this can be very verbose, so
many people don't.

ScopeGuard removes the boilerplate to make doing things the "right" way easier
than doing them the wrong way.

For example, imagine we want to set a GPIO on entry to a function, and reset it
on exit.

.. code-block:: c++
  :emphasize-lines: 2,5,10,16

  int the_function() {
    gpio.set();

    if (condition1) {
      gpio.reset();
      return 0;
    }
  
    if (condition2) {
      gpio.reset();
      return 1;
    }
      
    ...much code...
  
    gpio.reset();
    return 42;
  }

At any potential exit from the function, we have to reset the GPIO.  This is
troublesome:

- It may be hard for a reader or reviewer to convince themselves that it's
  done correctly.

- As a result, errors may creep in.

- Refactoring the function to have a single return point may cause it to be
  easier to read for these purposes, but harder to read for its original
  intent -- and doing so may be impossible if there are early exits due to
  exceptions or @ref ETL_CHECK.

The traditional C++ fix for this is to write a specialized class, perhaps
called ``ScopedGpio``, that has side effects when it's constructed (set) and
destroyed (reset).

.. code-block:: c++
  :emphasize-lines: 12,16

  class ScopedGpio {
  public:
    ScopedGpio(Gpio & gpio) : _gpio(gpio) {
      gpio.set();
    }
    ~ScopedGpio() {
      gpio.reset();
    }
  };

  int the_function() {
    ScopedGpio scoped_gpio(gpio);

    ... rest of body ...

    // GPIO gets reliably reset by scoped_gpio's destructor.
  }

This approach is robust: it will work on ``return``, on exception, whatever.
But we had to write some boilerplate.  If the result is reusable, great --- but
if it's a one-off, then that's an awful lot of typing to expect from a
beleaguered programmer!

Scope guard lets you get the same robustness without the typing:

.. code-block:: c++
  :emphasize-lines: 2,3

  int the_function() {
    gpio.set();
    ETL_ON_SCOPE_EXIT { gpio.reset(); };

    ... rest of body ...
  }

Any exit path from the scope *after* the use of :c:macro:`ETL_ON_SCOPE_EXIT`
will run the associated code.

Multiple Guards
---------------

Because the scope guard kicks in starting at the line where it's declared, a
scope can have *multiple* guards handling overlapping areas.  This is handy
when the setup process takes multiple steps and might fail along the way,
requiring only *part* of the cleanup to be run.

Here's an example taken from m4vgalib::

  void run() {
    vga::video_on();
    ETL_ON_SCOPE_EXIT { vga::video_off(); };

    vga::configure_band_list(&bands);
    ETL_ON_SCOPE_EXIT { vga::clear_band_list(); };

    while (true) {
      // main loop
    }
  }


Dismissing Guards
-----------------

But what if there is a path where we *don't* want the guard to do its job?
Maybe some other error handling already did it for us.  For such cases, we
can *dismiss* the guard.  To do this, we have to give it a name, using
:func:`make_guard` instead of :c:macro:`ETL_ON_SCOPE_EXIT`:

.. code-block:: c++
  :emphasize-lines: 3,6

  int the_function() {
    gpio.set();
    auto guard = etl::make_guard([] { gpio.reset(); });

    if (unusual_condition) {
      guard.dismiss();  // don't change the GPIO's state from here on.
    }

    ... rest of body ...
  }

After calling :func:`ScopeGuard<F>::dismiss()`, the guard is disabled along
every exit path from the scope, and the associated code won't be run.

In general, :c:macro:`ETL_ON_SCOPE_EXIT` is more concise, but
:func:`make_guard` is more flexible.


Compilation
-----------

The :c:macro:`ETL_ON_SCOPE_EXIT` macro generates code very similar to the
hand-rolled C-style equivalent using ``goto``.

:func:`make_guard` performs similarly unless you pass it an elaborate
function object with virtual methods or something.  The easiest way to do this
is by using ``std::function``; expect overhead if you do this.


API
---

Include::

    #include "etl/type_traits.h"

.. c:macro:: ETL_ON_SCOPE_EXIT

  Syntactic sugar for :class:`ScopeGuard` that declares an implicit lambda
  function.  Use like so::

    int the_function() {
      // Set the GPIO at entry to the function.
      gpio.set();
      // Make sure it gets reset when we leave this function.
      ETL_ON_SCOPE_EXIT { gpio.reset(); };

      ... rest of body ...
    }

.. function:: ScopeGuard<F> make_guard(F && fn)

  Given a callable object, such as a lambda function, function pointer, or
  functor object, produces a :class:`ScopeGuard` that stores the callable
  and calls it at destruction.

  In the normal use, you assign the result to a variable, so its destruction
  will coincide with the end of the current scope.  This is what makes it a
  "scope guard." ::

    {
      auto guard = etl::make_guard([]{ cleanup_action(); });
      // more code goes here
    }

.. class:: ScopeGuard<F>

  Holds a callable object of some type ``F`` and calls it at destruction.

  You'll rarely want to interact with the ScopeGuard type directly; use
  :func:`make_guard` to create one and ``auto`` to describe the result.
  This ensures that ScopeGuard can work with lambda functions, whose type
  cannot be written.

  .. function:: ScopeGuard(ScopeGuard<F> && source)

    Moves a ScopeGuard.  Since destroying a ScopeGuard has effects, they can't
    be *copied*, but they can be *moved*.  The ``source`` ScopeGuard is
    dismissed as though :func:`dismiss` had been called.

  .. function:: void dismiss()

    Disables the guard; its function will no longer be called.

    This is handy for cases where the guard's role is no longer relevant, e.g.
    if the guard was going to perform a cleanup action that's already been
    performed by some other error handling path.

  .. function:: ~ScopeGuard()

    The effect of the destructor depends on whether this guard has been
    dismissed (whether :func:`dismiss` has been called):

    - If the guard has *not* been dismissed, the associated function object is
      called with no arguments.

    - If the guard *has* been dismissed, nothing happens.
