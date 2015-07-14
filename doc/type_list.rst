TypeList
========

.. contents:: :local:
.. cpp:namespace:: etl


A representation for lists of types, and utilities for working with them.

This is directly inspired by Andrei Alexandrescu's %TypeList description from
*Modern C++ Design*.  The actual implementation is simplified to use C++11
variadic templates.

Do we need TypeList in C++11?
-----------------------------

Alexandrescu's implementation was written in terms of C++03.  Many of the
use cases for it, like creating a generic tuple class, were obviated by the
introduction of variadic templates in C++11.  As a result, it's tempting to
assume that we no longer have any need for TypeLists in this brave new
world.

Not so!  Variadic template parameter packs, while very flexible, are not
*first class* in the type system.  For example, a template instantiated
with a parameter pack cannot *re-export* its types easily::

  template <typename ... Ts>
  struct SomeStuff {
    using TypeArgs = Ts...;  // Can't do this.
  };

We need a type that stands in for the parameter pack.  That type is TypeList::

  template <typename ... Ts>
  struct SomeStuff {
    using TypeArgs = TypeList<Ts...>;  // Problem solved.
  };

Another case: class templates can only have one parameter pack in their
template parameter list::

  // Can't do this
  template <typename ... Xs, typename ... Ys>
  struct Processor {
    void process_xs(Xs...);
    void process_ys(Ys...);
  };

TypeList can wrap several parameter packs and keep them neatly separated,
at the cost of a specialization step::

  template <typename XList, typename YList>
  struct Processor;

  template <typename ... Xs, typename ... Ys>
  struct Processor<TypeList<Xs...>, TypeList<Ys...>> {
    void process_xs(Xs...);
    void process_ys(Ys...);
  };

Finally, because we can now refer to a list of types, we can define
operations over them::

  TypeList<char, ScopeGuard, int *>::contains<int *>()  // => true

API
---

Include::

  #include "etl/type_list.h"

.. cpp:class:: TypeList<T...>

  Represents a compile-time list of types, ``T...``.

  Because lists of types are not first-class, if you want to (say) take the list
  of types embodied by a TypeList and describe a function taking the same types
  as arguments, you must extract the variadic parameter pack by an additional
  template matching step.  Here is one such idiom::

    template <typename TL>
    class MyClass;

    template <typename ... T>
    class MyClass<TypeList<T...>> {
      void the_function(T && ... args) {
        // code here
      }
    };

  If you want to hide this from clients of your API, you can do so using
  inheritance; see the ETL vector templates for one example.  (TODO)

  .. cpp:member:: static constexpr bool all_unique

    Are all members of the typelist unique?

    Note that this is an exact definition of "unique."  For example, ``int`` and
    ``int const`` are distinct types.  If this concerns you, try applying
    :type:`Map\<F>` with ``std::decay``::

        using MyList = TypeList<int, int const, int &>;
        static_assert(MyList::all_unique,
            "these look like different types.");

        static_assert(MyList::Map<std::decay>::all_unique == false,
            "std::decay collapses them");

  .. cpp:type:: At<i>

    The type in the typelist at position ``i`` (zero-based)::

      using Numbers = TypeList<int, long, double>;

      using Type0 = Numbers::At<0>;
        // => int
      using Type4 = Numbers::At<4>;
        // => Compile error: index out of range

  .. cpp:function:: static constexpr bool contains<X>()

    Does this TypeList contain the given type?

    This operation is also available as a value-template,
    :type:`Contains\<X>`.

    ::

      using Numbers = TypeList<int, long, double>;

      Numbers::contains<int>()  // => true
      Numbers::contains<void>()  // => false

  .. cpp:type:: Contains<X>

    Contains a ``bool value`` member indicating whether the TypeList contains
    type `X`.

    This operation is also available as a constexpr function,
    :func:`contains<X>()`.

    ::

      using Numbers = TypeList<int, long, double>;
      Numbers::Contains<int>::value  // => true
      Numbers::Contains<void>::value  // => false

  .. cpp:type:: FindFirst<P>

    The first type ``T`` in the list for which the expression ``P<T>::value`` is
    ``true``::

      using Numbers = TypeList<int, long, double>;
      using SomeFloat = Numbers::FindFirst<std::is_floating_point>;
        // => double
      using SomeInt = Numbers::FindFirst<std::is_integral>;
        // => int
      using SomePtr = Numbers::FindFirst<std::is_pointer>;
        // => Compile error: no such type

  .. cpp:function:: static constexpr size_t index_of<X>()

    Index of type ``X`` in the typelist.  If ``X`` is not actually *in* the
    typelist, a ``static_assert`` will fire at compile time.

  .. cpp:type:: Map<F>

    A typelist of the same length, but with each type ``T`` in the original list
    replaced by ``Invoke<F<T>>``::

      using Numbers = TypeList<int, long, double>;
      using Pointers = Numbers::Map<std::add_pointer>;
        // => TypeList<int *, long *, double *>

  .. cpp:function:: static constexpr size_t size()

    Returns the number of types in the typelist.

    This could just as well be a constant; it's a constexpr function by
    analogy to the structure of C++14's ``std::integer_sequence``, though I'm
    not sure why.

.. cpp:type:: Repeat<T, count>

  An alias for the :class:`TypeList` containing the type ``T`` repeated
  ``count`` times::

    using FiveInts = TypeList<int, int, int, int, int>;
    using RepeatedInt = Repeat<int, 5>;
    static_assert(std::is_same<FiveInts, RepeatedInt>::value,
        "the two are equivalent");
