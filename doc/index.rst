.. ETL documentation master file, created by
   sphinx-quickstart on Mon Jul 13 17:01:51 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

ETL: The Embedded Template Library
==================================

The ETL is my reusable collection of C++ code, primarily intended for embedded
systems, but useful in other contexts as well.

When the C++ standard library is available, the ETL is designed to complement
it.

But the C++ standard library is really designed for C++'s primary dialect,
where exceptions and dynamic memory allocation are available.  In embedded
environments, particularly those with hard-real-time constraints, it's not
unusual to disable both!  In such cases, the ETL is desigend to take the place
of the C++ standard library, providing more suitable abstractions and
implementations.

Modules
-------

The ETL documentation is divided into *modules* by the topic or purpose of each
chunk of code.  A module generally maps to a single header or namespace; a few
are "catch-all" modules containing several headers.

.. toctree::
   :maxdepth: 2

   Math: high-performance vectors and the like <math>
   Maybe: values that might not exist <maybe>
   Scope Guards: easier than goto <scope_guard>
   Type Lists: first-class lists of types <type_list>
