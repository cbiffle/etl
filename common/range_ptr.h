#ifndef ETL_COMMON_RANGE_PTR_H_
#define ETL_COMMON_RANGE_PTR_H_

#include "etl/common/algorithm.h"
#include "etl/common/attribute_macros.h"

#define ETL_IMPLICIT /* TODO */

namespace etl {
namespace common {

template <typename E>
class RangePtr {
public:

//  static_assert(!is_void<E>::value, "RangePtr cannot currently be used with "
//                                    "void.");

  /*
   * Creates a RangePtr from explicit base and extent.
   */
  constexpr inline RangePtr(E *base, unsigned count)
      : _base(base), _count(count) {}

  /*
   * Creates a RangePtr by capturing the bounds of a static array.
   */
  template <unsigned N, typename T>
  constexpr inline ETL_IMPLICIT
  RangePtr(T (&array)[N])
      : _base(&array[0]), _count(N) {}

  /*
   * Overload for zero-length static arrays.  This one won't happen
   * naturally.
   */
  template <typename T>
  constexpr inline ETL_IMPLICIT
  RangePtr(T (&array)[0])
      : _base(&array[0]), _count(0) {}

  /*
   * Creates an empty RangePtr.
   */
  constexpr inline RangePtr() : _base(0), _count(0) {}

  /*
   * Implicit conversion from a RangePtr<T> to a RangePtr<T const>.
   */
  constexpr inline ETL_IMPLICIT operator RangePtr<E const>() const {
    return RangePtr<E const>(_base, _count);
  }

  /*
   * Explicit conversion to a different type, with a possible reduction in
   * length.
   */
  template <typename T>
  constexpr inline explicit
  operator RangePtr<T>() const {
    return RangePtr<T>(reinterpret_cast<T *>(_base),
                       _count * sizeof(E) / sizeof(T));
  }

  /*
   * Returns the number of elements in the range.
   */
  constexpr inline unsigned length() { return _count; }

  /*
   * Checks whether this RangePtr describes no elements.
   */
  constexpr inline bool is_empty() const { return _count == 0; }

  /*
   * Gets a raw pointer to the first element.
   */
  constexpr inline E *base() { return _base; }

  /*
   * UNSAFE array accessor.
   */
  inline E &operator[](unsigned index) const {
    return _base[index];
  }

  inline RangePtr tail_from(unsigned start) {
    //panic_if(_count < start, "tail index out of range");
    return RangePtr(_base + start, _count - start);
  }

  inline RangePtr tail() {
    return tail_from(1);
  }

  inline RangePtr slice(unsigned start, unsigned length) {
    if (start > _count) return RangePtr();
    return RangePtr(&_base[start], min(_count - start, length));
  }

  bool contents_equal(RangePtr other) {
    if (_count != other._count) return false;

    for (unsigned i = 0; i < _count; ++i) {
      if (_base[i] != other[i]) return false;
    }

    return true;
  }

private:
  E *_base;
  unsigned _count;
};

}  // namespace common
}  // namespace etl

#endif  // ETL_COMMON_RANGE_PTR_H_
