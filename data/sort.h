#ifndef _ETL_DATA_SORT_H_INCLUDED
#define _ETL_DATA_SORT_H_INCLUDED

#include "etl/utility.h"
#include "etl/data/range_ptr.h"

namespace etl {
namespace data {

/*
 * Sorts the contents of a range in-place using quicksort and an arbitrary
 * comparison function.
 */
template <typename T, typename CompareFn>
void qsort(RangePtr<T> elements, CompareFn && less_fn) {
  using std::size_t;
  using etl::swap;

  if (elements.count() <= 1) return;

  auto pivot_index = elements.count() / 2;

  // Move pivot to end for now.
  swap(elements[pivot_index], elements[elements.count() - 1]);
  pivot_index = elements.count() - 1;

  // 'partition' will hold the index in the array of the first element (other
  // than the pivot) that is greater than or equal to the pivot.
  size_t partition = 0;

  for (size_t i = 0; i < pivot_index; ++i) {
    if (less_fn(elements[i], elements[pivot_index])) {
      swap(elements[i], elements[partition++]);
    }
  }

  // Move pivot into final position.
  swap(elements[partition], elements[pivot_index]);

  qsort(elements.first(partition), less_fn);
  qsort(elements.tail_from(partition + 1), less_fn);
}


}  // namespace data
}  // namespace etl

#endif  // _ETL_DATA_SORT_H_INCLUDED
