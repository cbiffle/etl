#ifndef _ETL_DATA_RANGE_PTR_H_INCLUDED
#define _ETL_DATA_RANGE_PTR_H_INCLUDED

#include <cstddef>

#include "etl/assert.h"
#include "etl/attribute_macros.h"
#include "etl/implicit.h"

namespace etl {
namespace data {

struct LaxRangeCheckPolicy;


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
 * of memory and access the contents of those regions.  Optionally, the accesses
 * can be bounds-checked at runtime by adjusting the policy choices.
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
template <typename E, typename Policy = LaxRangeCheckPolicy>
class RangePtr {
public:
  /*
   * Creates an empty RangePtr.
   */
  ETL_INLINE constexpr RangePtr() : _base(nullptr), _count(0) {}

  /*
   * Creates a RangePtr from explicit base and extent.
   *
   * Note that you should rarely need to do this.
   */
  ETL_INLINE constexpr RangePtr(E *base, std::size_t count)
      : _base(base), _count(count) {}

  /*
   * Creates a RangePtr by capturing the bounds of a static array.  Note that
   * this only matches for arrays of 1+ elements.
   */
  template <std::size_t N>
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
  ETL_INLINE constexpr std::size_t count() { return _count; }

  /*
   * Returns the number of bytes in the range.
   */
  ETL_INLINE constexpr std::size_t byte_length() {
    return count() * sizeof(E);
  }

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
   * Array accessor.
   */
  ETL_INLINE constexpr E &operator[](std::size_t index) const {
    return _base[Policy::check_index(index, _count)];
  }

  ETL_INLINE constexpr RangePtr slice(std::size_t start,
                                      std::size_t end) {
    return RangePtr(&_base[Policy::check_slice_start(start, end, _count)],
                    Policy::check_slice_end(start, end, _count));
  }

  ETL_INLINE constexpr RangePtr tail_from(std::size_t start) {
    return slice(start, _count - start);
  }

  ETL_INLINE constexpr RangePtr tail() {
    return tail_from(1);
  }

  ETL_INLINE constexpr RangePtr first(std::size_t count) {
    return slice(0, count);
  }

  ETL_INLINE E & pop_front() {
    E &result = (*this)[0];
    *this = slice(1, _count);
    return result;
  }

  bool contents_equal(RangePtr other) {
    if (_count != other._count) return false;

    for (std::size_t i = 0; i < _count; ++i) {
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
  std::size_t _count;
};

/*
 * This RangePtr checking policy will tolerate *anything.*  It is dangerous,
 * but efficient.
 */
struct LaxRangeCheckPolicy {
  static constexpr std::size_t check_index(std::size_t index,
                                           std::size_t) {
    return index;
  }

  static constexpr std::size_t check_slice_start(std::size_t start,
                                                 std::size_t,
                                                 std::size_t) {
    return start;
  }

  static constexpr std::size_t check_slice_end(std::size_t start,
                                               std::size_t end,
                                               std::size_t) {
    return end - start;
  }
};

/*
 * This RangePtr checking policy wil assert on any out-of-range access.
 */
struct AssertRangeCheckPolicy {
  static constexpr std::size_t check_index(std::size_t index,
                                           std::size_t count) {
    return ETL_ASSERT_CE(index < count), index;
  }

  static constexpr std::size_t check_slice_start(std::size_t start,
                                                 std::size_t end,
                                                 std::size_t count) {
    return ETL_ASSERT_CE(start < count), start;
  }

  static constexpr std::size_t check_slice_end(std::size_t start,
                                               std::size_t end,
                                               std::size_t count) {
    return ETL_ASSERT_CE(start <= end && end <= count), end;
  }
};

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_RANGE_PTR_H_INCLUDED
