#ifndef ETL_DATA_RANGE_PTR_H_
#define ETL_DATA_RANGE_PTR_H_

#include "etl/common/algorithm.h"
#include "etl/common/attribute_macros.h"
#include "etl/common/implicit.h"

namespace etl {
namespace data {

/*
 * A pointer to a bounded, contiguous range of values.
 *
 * RangePtr is intended to replace the C pattern of passing an (address, size)
 * pair to describe a section of memory.  The old pattern is both awkward and
 * dangerous.  Awkward, because it increases parameter counts.  Dangerous,
 * because nothing enforces that the parameters "go together," so nothing
 * prevents code from walking off the end of the region, or misinterpreting a
 * byte count as an element count or vice versa.
 *
 * RangePtr provides a syntactically convenient way to pass pointers to regions
 * of memory, and (optionally bounds-checked) ways of accessing the region.
 *
 * RangePtr defines an implicit conversion from statically sized arrays:
 *
 *   void process(RangePtr<uint8_t> data);
 *
 *   uint8_t my_data[42];
 *   process(my_data);  // works
 *
 * This makes it difficult to pass the wrong size, and eliminates the need
 * for clever array-length-finding macros.
 *
 * Like pointers, RangePtrs are intended to be passed around by value.  The
 * range can be *shrunk* using pop_front or slice, but never *grown* (except by
 * assignment from a larger RangePtr).
 */
template <typename E>
class RangePtr {
public:
  // TODO(cbiffle): this really wants size_t, but currently ETL doesn't
  // define an equivalent type.  This begins to matter on LP64 systems,
  // which I'm not currently targeting...

  /*
   * Creates an empty RangePtr.
   */
  ETL_INLINE constexpr RangePtr() : _base(nullptr), _count(0) {}

  /*
   * Creates a RangePtr from explicit base and extent.
   *
   * Note that you should rarely need to do this.
   */
  ETL_INLINE constexpr RangePtr(E *base, unsigned count)
      : _base(base), _count(count) {}

  /*
   * Creates a RangePtr by capturing the bounds of a static array.  Note that
   * this only matches for arrays of 1+ elements.
   */
  template <unsigned N>
  ETL_INLINE ETL_IMPLICIT constexpr RangePtr(E (&array)[N])
      : _base(&array[0]), _count(N) {}

  /*
   * Overload for zero-length static arrays.  Interestingly, such arrays don't
   * match the template above.
   */
  ETL_INLINE ETL_IMPLICIT constexpr RangePtr(E (&array)[0])
      : _base(&array[0]), _count(0) {}

  /*
   * Implicit conversion from a RangePtr<T> to a RangePtr<T const>.
   */
  ETL_INLINE ETL_IMPLICIT constexpr operator RangePtr<E const>() {
    return RangePtr<E const>(_base, _count);
  }

  /*
   * Explicit conversion to a different type, with a possible reduction in
   * length.
   */
  template <typename T>
  ETL_INLINE explicit constexpr operator RangePtr<T>() const {
    // TODO(cbiffle): potential for overflow in length calculation.
    return RangePtr<T>(reinterpret_cast<T *>(_base),
                       _count * sizeof(E) / sizeof(T));
  }

  /*
   * Returns the number of elements in the range.
   */
  ETL_INLINE constexpr unsigned count() { return _count; }

  /*
   * Returns the number of bytes in the range.
   */
  ETL_INLINE constexpr unsigned byte_length() { return count() * sizeof(E); }

  /*
   * Checks whether this RangePtr describes no elements.
   */
  ETL_INLINE constexpr bool is_empty() { return _count == 0; }

  /*
   * Gets a raw pointer to the first element.  From this point on, all
   * safety guarantees are void.
   */
  ETL_INLINE constexpr E *base() { return _base; }

  /*
   * UNSAFE array accessor.
   */
  ETL_INLINE E &operator[](unsigned index) const {
    return _base[index];
  }

  ETL_INLINE RangePtr slice(unsigned start, unsigned length) {
    // TODO(cbiffle): handling policy
    // TODO(cbiffle): this seems like it ought to take start/end indices.
    if (start > _count) return RangePtr();
    return RangePtr(&_base[start], ::etl::common::min(_count - start, length));
  }

  ETL_INLINE RangePtr tail_from(unsigned start) {
    return slice(start, _count - start);
  }

  ETL_INLINE RangePtr tail() {
    return tail_from(1);
  }

  bool contents_equal(RangePtr other) {
    if (_count != other._count) return false;

    for (unsigned i = 0; i < _count; ++i) {
      if (_base[i] != other[i]) return false;
    }

    return true;
  }

  template <typename X>
  ETL_INLINE constexpr bool operator==(RangePtr<X> other) {
    return _base == other.base()
        && byte_length() == other.byte_length();
  }

  template <typename X>
  ETL_INLINE constexpr bool operator!=(RangePtr<X> other) {
    return !(*this == other);
  }

private:
  E *_base;
  unsigned _count;
};

}  // namespace data
}  // namespace etl

#endif  // ETL_DATA_RANGE_PTR_H_
