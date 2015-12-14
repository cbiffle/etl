Maybe
=====

.. contents:: :local:
.. namespace:: etl::data

Discussion
----------

A :class:`Maybe\<T> <Maybe>` is a ``T``-sized chunk of memory
that may or may not contain useful data, plus a "full" flag indicating whether
it has been initialized.

Like :class:`RangePtr`, :class:`Maybe` is intended to replace common
error-prone constructs --- specifically, many uses of out-parameters, and cases
where you store both a value and a flag indicating whether it's valid.  And
like :class:`RangePtr`, :class:`Maybe` is designed to allow abstraction without
cost: it's designed to boil down to the code you would have written by
hand, with no penalty to code generation.


An Example
^^^^^^^^^^

For example, imagine a function that gets the latest altitude measurement, if
one is available.  Traditionally it might have the signature::

  bool get_altitude(float * a);

If an altitude is available, such a function would write through the ``a``
pointer and return ``true``.  If no altitude is available, it simply returns
``false``.

This is both awkward and dangerous:

.. code-block:: c++
  :emphasize-lines: 2,6,7

  void use_altitude_for_something() {
    float altitude;   // Awkward: separate declaration line required.
    if (get_altitude(&altitude)) {
      use_altitude(altitude);
    }
    // Dangerous: nothing prevents this:
    use_altitude_again(altitude);  // uninitialized memory!
  }

.. warning:: Passing a value by-address into a function defeats compiler
  "uninitialized value" warnings!

This pattern isn't even *possible* for types that have an expensive default
constructor, or none at all.

:class:`Maybe` provides a safer, more general way::

  Maybe<float> get_altitude();

If an altitude is available, this function returns it, wrapped in a
:class:`Maybe`.  If no altitude is available, however, it returns an *empty*
:class:`Maybe`.  The equivalent to our function above becomes::

  void use_altitude_for_something() {
    // No pre-declaration of variable required.
    Maybe<float> altitude = get_altitude();

    if (altitude) {
      // It isn't empty, it's safe to use.
      use_altitude(altitude.const_ref());
    }

    // This use can be configured to assert, but it
    // will also often trigger uninitialized value
    // warnings.
    use_altitude_again(altitude.const_ref());
  }


Abstraction Costs
^^^^^^^^^^^^^^^^^

Returning a :class:`Maybe\<T> <Maybe>` is often *cheaper* than writing through
a pointer parameter and returning a flag, thanks to C++ return value
optimization and efficient struct-return calling conventions.  This is
particularly true when :ref:`literal type optimizations <data-maybe-literal>`
allow the call to be optimized away.

The other side of cost is determined by the :ref:`data-maybe-policy`, which you
control.

.. _data-maybe-empty-and-full:

Empty and Full
^^^^^^^^^^^^^^

An empty :class:`Maybe\<T>` (known as :var:`nothing`) can be passed around
freely without ever invoking ``T``'s constructors or assignment operators ---
avoiding potentially expensive construction when the value is absent.

A full :class:`Maybe\<T>` ("something") guarantees that the ``T`` contained
within has been constructed, and will destruct the ``T`` properly when the
:class:`Maybe` itself is destroyed.


.. _data-maybe-literal:

Literal and Non-Literal Types
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

:class:`Maybe` is carefully designed to operate with both literal and
non-literal types.

For a literal type ``L``, :class:`Maybe\<L> <Maybe>` is a literal type, and all
of its (non-mutating) operations are ``constexpr``.  This means a lot more
optimization opportunities exposed to the compiler for common types like
``Maybe<int>``.  It also means that :class:`Maybe` instances can be stored in
ROM, to hold e.g. optional configuration parameters, and in C++17 they're usable
as template parameters.

For other types, :class:`Maybe`'s operations are ``constexpr`` where possible
without sacrificing semantics.  In particular, for resource-management classes
that do work in their constructors and destructors, :class:`Maybe` will ensure
that they are constructed and destroyed when appropriate.  This means
:class:`Maybe` is safe to use with objects like smart pointers and file
handles.


.. _data-maybe-nothing:

The Nothing
^^^^^^^^^^^

:class:`Maybe` has a special relationship with its sister type,
:class:`Nothing`, and its instance :var:`nothing`.  :class:`Nothing` is a tag
type that can be converted to an *empty* :class:`Maybe\<T>` for any ``T``, just
as ``nullptr`` can be converted to a null pointer to any type.


.. _data-maybe-policy:

Access Checking Policy
^^^^^^^^^^^^^^^^^^^^^^

If you attempt to access the ``T`` contained within an *empty* :class:`Maybe\<T>
<Maybe>`, ETL can be configured to assert.  This prevents the program from
accessing uninitialized memory, the source of many subtle bugs.

In conservatively structured code, with compiler optimization enabled, the
assertions *usually* do not make it into the generated code.  For example::

  Maybe<int> value = /* something */;
  if (value) {
    operate(value.const_ref());
  }

Because the fullness check passed, the compiler can see that the assertion in
:func:`const_ref <Maybe<T, CheckPolicy\>::const_ref>` cannot fire, and it gets compiled away.

However, sometimes assertions do appear in the generated code, causing runtime
overhead.  ETL, like C++, tries not to impose *mandatory* overhead, and so
these checks can be disabled, or even replaced, using a *policy class*.

When :class:`Maybe` is using a policy class, it is written :class:`Maybe\<T,
CheckingPolicy> <Maybe>`.  The ``CheckingPolicy`` must conform to the interface
described as :class:`MaybeCheckPolicy` below.  Two policies are included in ETL,
and you can (of course) supply your own.

- :class:`AssertMaybeCheckPolicy` uses :c:macro:`ETL_ASSERT_CE` to enforce
  access.  This means accesses that are statically bogus often produce a compile
  error instead of a runtime error.  But this can't be guaranteed, and this
  policy may result in runtime overhead.

- :class:`LaxMaybeCheckPolicy` lets you do anything you want, in the grand
  tradition of C.  It has no overhead and no seatbelts.

.. warning:: For historical reasons, if you don't specify a ``CheckingPolicy``,
  you get a default of :class:`LaxMaybeCheckPolicy`.

Because writing out the policy every time is awkward, you should consider making
the decision at a project- or module-level and embodying it in a type alias::

  namespace my_project {
    template <typename T>
    using Maybe = ::etl::data::Maybe<T, MyCheckPolicy>;
  }

This way, if you ever want to change policies --- or select a different policy
in release vs. debug builds --- you can do it in one place.

API
---

::

  #include "etl/data/maybe.h"

Maybe
^^^^^

.. _etl-data-maybe:
.. cpp:class:: Maybe<T, CheckPolicy>

  (The default value of ``CheckPolicy`` is :class:`LaxMaybeCheckPolicy`.  This
  is not shown above due to technical limitations.)

  A wrapper for a ``T`` that may or may not be present.

  :class:`Maybe\<T> <Maybe>` physically contains the ``T``, it is not (for
  example) heap-allocated.  Of course, ``T`` can be a pointer or smart-pointer
  type tracking something on the heap if desired.

  :class:`Maybe\<T>` becomes a literal type if ``T`` is a literal type.  (See
  :ref:`data-maybe-literal` for details.)  The operations below that become
  ``constexpr`` for literal types are marked with the phrase "``constexpr`` for
  literal ``T``".  Operations that are *always* ``constexpr`` will have it in
  their signatures (e.g.  :func:`is_something`).

  You have the option of ignoring, asserting, or doing something else (e.g.
  throwing an exception) if a program attempts to access the contents of an
  empty maybe, by your selection of the ``CheckPolicy``.  See
  :ref:`data-maybe-policy` for details.

  .. cpp:function:: constexpr Maybe()
  .. cpp:function:: constexpr Maybe(Nothing)

    Creates an empty maybe.

  .. cpp:function:: explicit Maybe<A...>(InPlace, A && ... args)

    Creates a full maybe containing a ``T`` constructed using the given
    ``args``.  (:class:`InPlace` is acting as a tag here.)

    ``constexpr`` for literal ``T``.

  .. cpp:function:: Maybe(T const &)
  .. cpp:function:: Maybe(T &&)
  .. cpp:function:: Maybe(S const &)
  .. cpp:function:: Maybe(S &&)

    Creates a full maybe by copying or moving a value, which can either be a
    plain old ``T``, or a value of any type ``S`` that is assignable to ``T``::

      Maybe<int> an_int(3);       // Exact match.
      Maybe<double> a_double(3);  // '3' is an int, but it's assignable.

    ``constexpr`` for literal ``T`` (and, where necessary, ``S``).

  .. cpp:function:: Maybe(Maybe<T> const &)
  .. cpp:function:: Maybe(Maybe<T> &&)
  .. cpp:function:: Maybe<S>(Maybe<S> const &)
  .. cpp:function:: Maybe<S>(Maybe<S> &&)

    Creates a maybe by copying or moving the given maybe, which can be either
    the same type ``Maybe<T>``, or a some type ``Maybe<S>`` where ``S`` is
    assignable to ``T``.  That might be a mouthful, but it'll pretty much just
    do what you'd expect::

      Maybe<int> an_int(3);
      Maybe<int> another_int(an_int);  // Works, exact match.
      Maybe<double> a_double(an_int);  // Works, assignable.

    The result will be full if (and only if) the input maybe was full.

  .. cpp:function:: ~Maybe()

    If full, destroys the contained ``T``.  If empty, does nothing.

    The destructor is omitted for literal ``T``.

  .. cpp:function:: Maybe<T> & operator=(Maybe<S> const & input)
  .. cpp:function:: Maybe<T> & operator=(Maybe<S> && input)

    Replaces the contents of a maybe by copy-assignment or move-assignment
    from another maybe.  ``S`` must be assignable to ``T``.

    The implementation jumps through some hoops to try to do what you expect
    under the hood:

    - If the target is full and the ``input`` is empty, the contents of the
      target are destroyed.

    - If the target is full and the ``input`` is full, we defer to ``T``'s
      assignment operator and assign the contents of ``input``.

    - If the target is empty and the ``input`` is full, we copy- or
      move-construct the contents of the target in place.

    - If the target is empty and the ``input`` is empty, nothing happens.

    In any case, at the end, the target is full if (and only if) the ``input``
    is.

  .. cpp:function:: void clear()
  .. cpp:function:: Maybe<T> & operator=(Nothing)

    Makes a maybe become empty, destroying its contents if necessary.  Note
    that this can be written either as an explicit ``clear()`` or using
    an assignment from :var:`nothing`.

  .. cpp:function:: constexpr bool is_something() const
  .. cpp:function:: constexpr explicit operator bool() const

    Does this maybe contain something?

  .. cpp:function:: constexpr bool is_nothing() const

    Is this maybe empty?

  .. cpp:function:: constexpr T const & const_ref() const

    Gets a const reference to the contents of a maybe.

    If the maybe is empty, you have done it wrong.  The ``CheckingPolicy``'s
    :func:`check_access <MaybeCheckPolicy::check_access>` function is invoked
    to decide how to handle this.

  .. cpp:function:: T & ref()

    Gets a non-const reference to the contents of a maybe.

    If the maybe is empty, you have done it wrong.  The ``CheckingPolicy``'s
    :func:`check_access <MaybeCheckPolicy::check_access>` function is invoked
    to decide how to handle this.

.. cpp:function:: bool operator==(Maybe<T> const &, Maybe<S> const &)
.. cpp:function:: bool operator!=(Maybe<T> const &, Maybe<S> const &)

  Comparison for maybes.  The operator is available if the corresponding
  operator is defined for ``T`` and ``S``.

  - Any two empty maybes are equal.

  - An empty maybe is not equal to any full maybe.

  - Two full maybes are equal if (and only if) their contents are equal.

  ``constexpr`` for literal ``T`` and ``S``.

.. cpp:function:: bool operator==(Maybe<T> const &, S const &)
.. cpp:function:: bool operator==(S const &, Maybe<T> const &)
.. cpp:function:: bool operator!=(Maybe<T> const &, S const &)
.. cpp:function:: bool operator!=(S const &, Maybe<T> const &)

  Comparison of maybes with non-maybe types (the type ``S`` must not be a
  maybe type).  These operators are available if the corresponding operator is
  defined for ``T`` and ``S``.

  A non-maybe value is treated as a full maybe of the same type:

  - An empty maybe is not equal to it.

  - A full maybe is equal to it if (and only if) the maybe's contents are equal
    to it.

  ``constexpr`` for literal ``T`` and ``S``.

.. cpp:function:: constexpr bool operator==(Maybe<T> const &, Nothing)
.. cpp:function:: constexpr bool operator==(Nothing, Maybe<T> const &)
.. cpp:function:: constexpr bool operator!=(Maybe<T> const &, Nothing)
.. cpp:function:: constexpr bool operator!=(Nothing, Maybe<T> const &)

  Comparison of maybes with the universal :var:`nothing` value.  This is
  equivalent to :func:`is_nothing <Maybe<T, CheckPolicy\>::is_nothing>` (for
  ``==``) and :func:`is_something <Maybe<T, CheckPolicy\>::is_something>` (for
  ``!=``).


Placeholder Types
^^^^^^^^^^^^^^^^^

.. cpp:class:: Nothing

  The type of the :var:`nothing` value.

.. cpp:var:: static constexpr Nothing nothing

  A predefined instance of the :class:`Nothing` type.  Can be implicitly
  converted into an empty maybe of any type::

    Maybe<int> an_int = nothing;

.. cpp:class:: InPlace

  The type of the :var:`in_place` value.

.. cpp:var:: static constexpr InPlace in_place

  A predefined instance of the :class:`InPlace` type.  A tag used to
  disambiguate :class:`Maybe<T>`'s in-place value constructor from other
  constructors.

Check Policies
^^^^^^^^^^^^^^

.. cpp:class:: MaybeCheckPolicy

  **This isn't a real class.**  It's a template for what your checking policy
  classes ought to look like.

  .. cpp:function:: static constexpr X check_access(bool is_something)

    Called to check access attempts.  The return type is immaterial; it's
    ignored.

    If you implement this in a way that isn't ``constexpr`` (e.g. it uses
    ``throw`` recklessly) then certain :class:`Maybe` operations, particularly
    :func:`const_ref <Maybe<T, CheckingPolicy\>>`, will lose their ``constexpr``
    status even for literal types.

.. cpp:class:: LaxMaybeCheckPolicy

  An implementation of :class:`MaybeCheckPolicy` that handles access to
  uninitialized memory in the traditional C way: by silently succeeding.

  This is the policy to use if you're terrified of :class:`Maybe<T>`'s check
  overhead and are confident that your code will never have bugs.

  It is also, for historical reasons, the default.

  .. cpp:function:: static constexpr bool check_access(bool is_something)

    Does nothing.

.. cpp:class:: AssertMaybeCheckPolicy

  An implementation of :class:`MaybeCheckPolicy` that uses
  :c:macro:`ETL_ASSERT_CE` to prevent accesses to uninitialized memory.

  .. cpp:function:: static constexpr bool check_access(bool is_something)

    Asserts on ``is_something``.
