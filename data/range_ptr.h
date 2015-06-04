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
  /*****************************************************************
   * Making RangePtrs
   */

  /*
   * Creates an empty RangePtr.
   */
  ETL_INLINE constexpr RangePtr() : _base(nullptr), _count(0) {}

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
   * Creates a RangePtr from explicit base and extent.
   *
   * Note that you should rarely need to do this.  The array overloads above
   * provide a safer and more convenient alternative.
   */
  ETL_INLINE constexpr RangePtr(E *base, std::size_t count)
      : _base(base), _count(count) {}

  /*
   * Creates a RangePtr from explicit begin and end pointers.
   *
   * Note that you should rarely need to do this.  The array overloads above
   * provide a safer and more convenient alternative.
   */
  ETL_INLINE constexpr RangePtr(E *begin, E const *end)
      : _base(begin), _count(end - begin) {}

  /*
   * Implicit conversion from a RangePtr<T> to a RangePtr<T const>.
   */
  ETL_INLINE ETL_IMPLICIT constexpr operator RangePtr<E const>() const {
    return RangePtr<E const>(_base, _count);
  }

  /*****************************************************************
   * Basic access
   */

  /*
   * Returns the number of elements in the range.
   */
  ETL_INLINE constexpr std::size_t count() const { return _count; }

  /*
   * Returns the number of bytes in the range.
   */
  ETL_INLINE constexpr std::size_t byte_length() const {
    return count() * sizeof(E);
  }

  /*
   * Checks whether this RangePtr describes no elements.
   */
  ETL_INLINE constexpr bool is_empty() const { return _count == 0; }

  /*
   * Element accessor.  Note that while this supports an array-like syntax, it
   * may imply bounds-checked access, depending on the Policy.
   */
  ETL_INLINE constexpr E &operator[](std::size_t index) const {
    return _base[Policy::check_index(index, _count)];
  }

  /*
   * Shortens this range in-place by removing the first element.  Returns a
   * reference to the removed element.
   *
   * This function is useful when processing the range left-to-right, either in
   * a loop or in other code.
   *
   * This may be bounds-checked, depending on the Policy.
   */
  ETL_INLINE E & pop_front() {
    E &result = (*this)[0];
    *this = slice(1, _count);
    return result;
  }

  /*
   * Checks whether the given pointer falls within this range.
   */
  ETL_INLINE constexpr bool contains(void * ptr) const {
    return (_base <= ptr) && (ptr < &_base[_count]);
  }

  /*****************************************************************
   * Deriving new RangePtrs
   */

  /*
   * Returns a sub-range between start (inclusive) and end (exclusive).  This is
   * the most general derivation operation, but sometimes not the most clear;
   * see the operations below.
   *
   * This may be bounds-checked, depending on the Policy.
   */
  ETL_INLINE constexpr RangePtr slice(std::size_t start,
                                      std::size_t end) const {
    return RangePtr(&_base[Policy::check_slice_start(start, end, _count)],
                    Policy::check_slice_end(start, end, _count));
  }

  /*
   * Returns a "tail" of this range -- all elements starting at a certain index.
   *
   * This may be bounds-checked, depending on the Policy.
   */
  ETL_INLINE constexpr RangePtr tail_from(std::size_t start) const {
    return slice(start, _count);
  }

  /*
   * Returns all elements of this range after the first.
   *
   * This may be bounds-checked, depending on the Policy.
   */
  ETL_INLINE constexpr RangePtr tail() const {
    return tail_from(1);
  }

  /*
   * Returns the first N elements of this range.
   *
   * This may be bounds-checked, depending on the Policy.
   */
  ETL_INLINE constexpr RangePtr first(std::size_t count) const {
    return slice(0, count);
  }

  /*****************************************************************
   * Comparison
   */

  /*
   * Compares two RangePtrs for equality.  RangePtr equality is like pointer
   * equality -- in that both ranges must start at the same address to be equal
   * -- but the length of the ranges must also match.
   */
  template <typename X>
  ETL_INLINE constexpr bool operator==(RangePtr<X, Policy> other) const {
    return _base == other.base()
        && byte_length() == other.byte_length();
  }

  /*
   * Compares two RangePtrs for inequality.  RangePtr equality is like pointer
   * equality -- in that both ranges must start at the same address to be equal
   * -- but the length of the ranges must also match.
   */
  template <typename X>
  ETL_INLINE constexpr bool operator!=(RangePtr<X, Policy> other) const {
    return !(*this == other);
  }

  /*
   * Checks for "deep equality" of two RangePtrs -- that is, whether the ranges
   * they reference contain equivalent data.
   *
   * The two ranges must be equal length for the comparison to succeed.
   */
  bool contents_equal(RangePtr other) const {
    if (_count != other._count) return false;

    for (std::size_t i = 0; i < _count; ++i) {
      if (_base[i] != other[i]) return false;
    }

    return true;
  }

  /*****************************************************************
   * Conversions
   */

  /*
   * Explicit conversion to a different type, with a possible reduction in
   * element count.
   *
   * TODO(cbiffle): provide a version of this that uses static_cast for safety.
   */
  template <typename T>
  ETL_INLINE explicit constexpr operator RangePtr<T>() const {
    // TODO(cbiffle): potential for overflow in length calculation.
    return RangePtr<T>(reinterpret_cast<T *>(_base),
                       _count * sizeof(E) / sizeof(T));
  }

  /*
   * Gets a raw pointer to the first element.  Note that this allows arbitrary
   * unsafe pointer arithmetic  and should be used sparingly.
   */
  ETL_INLINE constexpr E *base() const { return _base; }

  /*****************************************************************
   * "Range-based for" and standard algorithm support
   *
   * These operations provide an adapter that allows a RangePtr to
   * be used as a traditional C++ iterator.  If the RangePtr is
   * checked, iteration over it will also be checked.
   */

  class Iterator {
  public:
    constexpr Iterator(RangePtr const & r) : _r(r) {}
    constexpr E & operator*() const { return _r[0]; }

    Iterator & operator++() {
      _r.pop_front();
      return *this;
    }

    constexpr bool operator!=(Iterator const & other) const {
      return _r != other._r;
    }

  private:
    RangePtr _r;
  };

  constexpr Iterator begin() const {
    return Iterator(*this);
  }

  constexpr Iterator end() const {
    return Iterator(slice(_count, _count));
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
    return ETL_ASSERT_CE(start <= count), start;
  }

  static constexpr std::size_t check_slice_end(std::size_t start,
                                               std::size_t end,
                                               std::size_t count) {
    return ETL_ASSERT_CE(start <= end && end <= count), end - start;
  }
};

/*
 * A convenient way to capture a static array as a RangePtr, in a context where
 * the implicit conversion constructor doesn't apply.
 */
template <typename T, size_t n>
inline RangePtr<T> range(T (&arr)[n]) {
  return RangePtr<T>(arr);
}

}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_RANGE_PTR_H_INCLUDED
