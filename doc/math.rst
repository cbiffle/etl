Vector Math
===========

Discussion
----------

Vectors
^^^^^^^

Matrices
^^^^^^^^

.. _math-promotion:

Types and Promotion
^^^^^^^^^^^^^^^^^^^


API
---

Vector
^^^^^^

Orientation
"""""""""""

.. cpp:enum-class:: Orient

  Used to distinguish the two orientations of vectors.

  .. cpp:enumerator:: col

    Indicates a column vector.  This is the default (see
    :ref:`math-column-vectors`).

  .. cpp:enumerator:: row

    Indicates a row vector.  These are relatively unusual in most applications;
    the main place they appear here is as the :type:`Row <Matrix<rows, cols,
    T\>::Row>` of a :type:`Matrix`.

.. cpp:function:: constexpr Orient flip(Orient)

  Evaluates to the opposite orientation::

    static_assert(flip(Orient::row) == Orient::col,
        "row and col are opposed");

Type
""""

.. cpp:class:: Vector<dim, T, orient>

  A vector of dimension ``dim`` containing elements of type ``T``.  It may be
  either a row or column vector, distinguished by ``orient`` (a value of type
  :enum:`Orient`); if not provided, the default is :enumerator:`Orient::col`.

  .. cpp:var:: static constexpr size_t dim

    The dimension (number of elements) of the vector.  This is the same as
    the first template parameter.

  .. cpp:var:: static constexpr Orient orient

    The orientation of the vector.  This is the same as the sceond template
    parameter.

  .. cpp:type:: T Element

    The element type of the vector.

  .. cpp:type:: Vector<dim, T, flip(orient)> Transposed

    The type of vectors with the same dimension and element type, but flipped.

  .. cpp:type:: Vector<dim, S, orient> WithType<S>

    A template alias for vectors with the same dimension and orientation, but
    with the element type replaced by ``S``.

  .. cpp:function:: constexpr Vector(T const & e1, T const & e2, ...)

    Constructs a vector by elements.  This is the main way of writing literal
    vectors using initializer-list syntax::

      constexpr rotate_ccw(Vec2f v) {
        return {-v.y, v.x};
      }

      rotate_ccw({3, 4});

    Don't let the signature above fool you: this is not a C-style varargs
    function.  It's only shown that way for brevity.  You must provide exactly
    :var:`dim` elements, no more, no less.

  .. _math-vector-repetition:

  .. cpp:function:: constexpr explicit Vector(T const &)

    Constructs a vector by repeating a single element :var:`dim` times::

      static_assert(Vec3f{3} == Vec3f{3, 3, 3},
          "element repeats into every dimension");

  .. cpp:function:: constexpr explicit Vector(Transposed const &)

    Transposition constructor.  This provides an alternative syntax to
    :func:`transposed`.

  .. cpp:var:: T x

  .. cpp:var:: T y

  .. cpp:var:: T z

  .. cpp:var:: T w

    Field names for the scalars of the first four dimensions, in vectors of
    dimension 4 and under::

      constexpr auto v1 = Vec4f{1, 2, 3, 4};
      constexpr auto v2 = Vec2f{v1.x, v1.w};
      constexpr auto v3 = Vec2f{v1.y, v1.z};
      static_assert(v1 == Vec4f{v2.x, v3.x, v3.y, v2.y},
          "fields are x-y-z-w in that order");

    For 5-vectors and larger, use the :var:`elements` array.

    To ignore this distinction, use the :func:`get` template.

  .. cpp:Var:: T elements[dim]

    Element array for vectors of dimension 5 and up.

    For 4-vectors and smaller, the fields can be accessed as :var:`x` through
    :var:`w`.

    To ignore this distinction, use the :func:`get` template.

Element Access
""""""""""""""

.. cpp:function:: constexpr T get<d>(Vector<dim, T, orient> const &)

  Evaluates to the scalar from dimension ``d`` of the vector (0-based).

  It is a compile error if ``d`` is out of range for a vector with ``dim``
  dimensions.

.. cpp:function:: constexpr Vector<n, T, orient> get<d1, d2, ...>(\
        Vector<dim, T, orient> const &)

  Vector shuffle operator.  Given ``n`` dimension numbers ``d1``, ``d2``,
  ... ``dn-1``, returns a vector of ``n`` dimensions where each element is
  taken from the corresponding dimension of the input::

      static_assert(get<2, 1, 0>(Vec3f{1, 2, 3}) == Vec3f{3, 2, 1},
          "reversing a vector");

  Dimensions can be repeated::

      static_assert(get<0, 0, 0>(Vec2f{1, 2}) == Vec3f{1, 1, 1},
          "dimensions can be repeated");

Basic Operations
""""""""""""""""

.. cpp:function:: constexpr Vector<d, T, flip(o)> \
        transposed(Vector<d, T, o> const &)

  A vector with the same dimension, element type, and contents as the input,
  but with opposed orientation.

.. cpp:function:: constexpr R dot(Vector<d, T, o> const &, \
                                  Vector<d, S, o> const &)

  Dot product of two vectors.  ``R`` is the type of multiplication of ``T`` and
  ``S``; see :ref:`math-promotion` for details.

.. cpp:function:: constexpr Vector<d, R, o> sqmag(Vector<d, T, o> const &)

  The square of the :func:`magnitude <mag>` of a vector, i.e. its elements
  squared and summed.  This is cheaper to compute than the magnitude and can
  sometimes play the same role.

  .. math::
  
    sqmag(\vec{v}) = \sum_{i=0}^{d-1} \vec{v}_i^2

    sqmag(\left[ x, y, z \right]) = x^2 + y^2 + z^2

  ``R`` is the type of multiplication of ``T`` and ``S``; see
  :ref:`math-promotion` for details.

.. cpp:function:: constexpr Vector<d, R, o> mag(Vector<d, T, o> const &)

  The magnitude of a vector, defined as its length in Euclidean space by the
  Pythagorean theorem.

  .. math::

    mag(\vec{v}) = \sqrt{sqmag(\vec{v})} = \sqrt{\sum_{i=0}^{d-1} \vec{v}_i^2}

    mag(\left[ x, y, z \right]) = \sqrt{x^2 + y^2 + z^2}

Arithmetic Operations
"""""""""""""""""""""

The types of the arithmetic operators are somewhat complex due to
:ref:`promotion <math-promotion>`.

.. cpp:function:: constexpr Vector<d, R, o> operator-(Vector<d, T, o> const &)

  Vector negation; negates each element.

.. cpp:function:: constexpr Vector<d, R, o> operator+(Vector<d, T, o> const &, \
                                                      Vector<d, S, o> const &)
.. cpp:function:: constexpr Vector<d, R, o> operator-(Vector<d, T, o> const &, \
                                                      Vector<d, S, o> const &)

  Vector addition/subtraction.

  Note that you cannot directly add or subtract vectors and scalars.  If you'd
  like to add a number to each *element* of a vector, you can use
  :ref:`repetition syntax <_CPPv2N6VectorI3dim1T6orientE6VectorER1T>`::

      Vec3f{1, 2, 3} + Vec3f{2}  // adds 2 to each element

.. cpp:function:: constexpr Vector<d, R, o> operator*(Vector<d, T, o> const &, \
                                                      S const &)
.. cpp:function:: constexpr Vector<d, R, o> operator*(S const &, \
                                                      Vector<d, T, o> const &)
.. cpp:function:: constexpr Vector<d, R, o> operator/(Vector<d, T, o> const &, \
                                                      S const &)
.. cpp:function:: constexpr Vector<d, R, o> operator/(S const &, \
                                                      Vector<d, T, o> const &)

  Multiplying/dividing vectors and scalars.

  Note that neither multiplication nor division is defined for two vectors,
  but see :func:`parallel_mul`.

.. cpp:function:: constexpr Vector<d, R, o> parallel_mul(\
                            Vector<d, A, o> const &, \
                            Vector<d, B, o> const &)

  The vector computed by multiplying corresponding elements of the two input
  vectors.

Type Aliases
""""""""""""

.. cpp:type:: Vector<2, T, orient> Vec2<T, orient>
.. cpp:type:: Vector<3, T, orient> Vec3<T, orient>
.. cpp:type:: Vector<4, T, orient> Vec4<T, orient>

  Template aliases for common sizes of vector.  As usual, if ``orient`` is
  omitted, the default is :enumerator:`Orient::col`.

.. cpp:type:: Vec2<float> Vec2f
.. cpp:type:: Vec3<float> Vec3f
.. cpp:type:: Vec4<float> Vec4f

  Type aliases for common sizes of single-precision floating point vector.

.. cpp:type:: Vec2<int> Vec2i
.. cpp:type:: Vec3<int> Vec3i
.. cpp:type:: Vec4<int> Vec4i

  Type aliases for common sizes of integer floating point vector.

Matrix
^^^^^^

Type
""""

.. cpp:class:: Matrix<rows, cols, T>

  A ``rows``-by-``cols`` matrix containing elements of type ``T``.

  Certain common C++ operations, such as copy/move constructors, assignment
  operators, and the like, are omitted below.

  .. cpp:type:: Vector<cols, T, Orient::row> Row 

    The type of one row of this matrix --- a row-vector with the same number
    of columns and element type.

  .. cpp:var:: Row row[rows]

    The array of rows making up this matrix.

  .. cpp:var:: static constexpr size_t rows

    Number of rows in this matrix type.

  .. cpp:var:: static constexpr size_t cols

    Number of columns in this matrix type.

  .. cpp:var:: static constexpr bool square

    Is this a square matrix?  ``true`` only if ``rows == cols``.

  .. cpp:type:: T Element

    Element type of this matrix (aka ``T``).

  .. cpp:function:: constexpr Matrix()

    The usual constructor suspects.

  .. cpp:function:: constexpr Matrix(Row const & r1, Row const & r2, ...)

    Creates a matrix from some existing rows.  This is the main way of writing
    literal matrices using initializer-list syntax::

      constexpr Mat2f rotate(float angle) {
        return {
          {cos(a), -sin(a)},
          {sin(a),  cos(a)},
        };
      }

    Don't let the signature above fool you: this is not a C-style varargs
    function.  It's only shown that way for brevity.  You must provide exactly
    ``rows`` rows, no more, no less.

  .. cpp:function:: constexpr static Matrix identity()

    Returns the identity matrix for a matrix type.

    Only available on square matrices.


Operations
""""""""""

.. cpp:function:: constexpr E get<row, col>(Matrix<r, c, E> const &)

  Evaluates to the element at position (``row``, ``col``), both 0-based.  The
  position is known at compile time and passed as template parameters::

    constexpr auto m = Mat2f {
      {2, 3},
      {6, 7},
    };
    static_assert(get<0, 0>(m) == 2, "upper left");
    static_assert(get<1, 0>(m) == 6, "lower left");
    static_assert(get<1, 1>(m) == 7, "lower right");

  If the element position is not defined for a matrix of this size, it's a
  compile error.

.. cpp:function:: constexpr Matrix<c, r, T> transposed(Matrix<r, c, T> const &)

  Evaluates to the transposition of the input matrix::

    constexpr auto m = Mat2f {
      {2, 3},
      {6, 7},
    };

    auto m_transposed = Mat2f {
      {2, 6},
      {3, 7},
    };

    static_assert(m == m_transposed, "");


.. cpp:function:: constexpr bool operator==(Matrix<r, c, T> const &, \
                                            Matrix<r, c, S> const &)
.. cpp:function:: constexpr bool operator!=(Matrix<r, c, T> const &, \
                                            Matrix<r, c, S> const &)

  Comparison of like-sized matrices with element types ``T`` and ``S``;
  available if the corresponding operator is defined for ``T`` and ``S``.

.. cpp:function:: constexpr Vector<n, R> operator*(Matrix<n, n, M> const &, \
                                                   Vector<n, V> const &)

  Multiplies a square matrix by a column vector, giving a new column vector.

  ``R`` (the element type of the result) is the result of multiplying values of
  type ``M`` by type ``V``.  See :ref:`math-promotion` for more details.

  Notes:

  - This could be generalized to non-square matrices, except I haven't needed
    that yet.

  - For the same reason, multiplication by row vectors is not yet implemented.

.. cpp:function:: constexpr Matrix<n, p, C> operator*(Matrix<n, m, A> const &, \
                                                      Matrix<m, p, B> const &)

  Multiplies an *n* by *m* matrix by an *m* by *p* matrix, yielding an *n* by
  *p* result matrix.

  ``C`` (the element type of the result) is the result of multiplying values of
  type ``A`` by type ``B``.  See :ref:`math-promotion` for more details.

Type Aliases
""""""""""""

.. cpp:type:: Matrix<2, 2, T> Mat2<T>
.. cpp:type:: Matrix<3, 3, T> Mat3<T>
.. cpp:type:: Matrix<4, 4, T> Mat4<T>

  Template aliases for common sizes of square matrix.

.. cpp:type:: Mat2<float> Mat2f
.. cpp:type:: Mat3<float> Mat3f
.. cpp:type:: Mat4<float> Mat4f

  Type aliases for common sizes of single-precision floating-point matrix.
