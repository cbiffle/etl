#include "etl/mem/arena.h"

#include "etl/assert.h"
#include "etl/bits.h"

using etl::data::Maybe;
using etl::data::RangePtr;

namespace etl {
namespace mem {

/*
 * Utility function: rounds up to the next multiple of n, where n must be
 * a power of two.
 */
template <std::size_t n>
static constexpr std::uintptr_t round_up(std::uintptr_t bits) {
  return ETL_ASSERT_CE((n & (n - 1)) == 0), (bits + (n - 1)) & ~(n - 1);
}

/*
 * Utility function: rounds up a *pointer* to the next multiple of n bytes,
 * where n must e a power of two.  Disregards the pointer's natural alignment.
 */
template <std::size_t n, typename T>
static constexpr T * round_up(T * ptr) {
  return reinterpret_cast<T *>(
      round_up<n>(reinterpret_cast<std::uintptr_t>(ptr)));
}

Arena::Arena(RangePtr<Region const> table)
  : _prototype(table),
    _table(),
    _total_bytes(0),
    _free_bytes(0) {}

void Arena::reset() {
  // We need to steal some memory from the first region for our bookkeeping.
  // Make sure that'll work...
  auto region_count = _prototype.count();
  if (region_count == 0) return;

  // TODO(cbiffle): drop bytes from beginning to achieve alignment.

  auto table_size = sizeof(Region) * region_count;
  ETL_ASSERT(_prototype[0].byte_length() >= table_size);

  // Yay!

  // Point our RAM table at the bottom section of the first region.
  // Double-casting nonsense deals with alignment change.
  _table = {
    static_cast<Region *>(static_cast<void *>(_prototype[0].base())),
    region_count
  };

  // Now copy the ROM table into RAM, accumulating the total byte count.
  _total_bytes = 0;

  // TODO(cbiffle): align the regions to our minimum allocation granule as they
  // are copied.
  for (unsigned i = 0; i < region_count; ++i) {
    _table[i] = _prototype[i];
    _total_bytes += _table[i].byte_length();
  }

  // Adjust first entry to account for what we've used.
  _table[0] = _table[0].tail_from(table_size);
  _free_bytes = _total_bytes - table_size;
}

void * Arena::allocate(std::size_t bytes) {
  bytes = round_up<4>(bytes);

  // Take space from the first region that can satisfy the request.
  for (auto & region : _table) {
    if (region.byte_length() >= bytes) {
      void * p = region.base();
      region = region.tail_from(bytes);
      _free_bytes -= bytes;
      return p;
    }
  }

  // Allocation failed.
  ETL_ASSERT(false);
}

}  // namespace mem
}  // namespace etl
