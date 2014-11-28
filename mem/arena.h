#ifndef _ETL_MEM_ARENA_H_INCLUDED
#define _ETL_MEM_ARENA_H_INCLUDED

/*
 * A basic "arena" memory manager, factored out of m4vgalib.
 *
 * This is somewhat more flexible than many C++ region-based allocators, because
 * it has to deal with weird non-contiguous memory architectures of the sort we
 * find in embedded SoCs.
 *
 * The Arena is initialized with a table of memory regions that it manages.
 * When an allocation request arrives, it satisfies it from the first region in
 * the table with remaining space.  This lets applications arrange for a
 * particular memory layout by coordinating the table structure and the
 * allocation order.  m4vgalib does this extensively.
 */

#include <cstddef>
#include <cstdint>

#include "etl/data/maybe.h"
#include "etl/data/range_ptr.h"

namespace etl {
namespace mem {

/*
 * A convenient type for describing a range of memory, modeled using
 * RangePtr.
 */
using Region = etl::data::RangePtr<std::uint8_t>;

/*
 * A region-based memory allocator.
 */
class Arena {
public:
  /*
   * Creates an Arena using the given range of Regions, which may be in ROM.
   * The Arena will deny allocation requests until it has been reset().
   */
  Arena(etl::data::RangePtr<Region const>);

  /*
   * Restores this Arena to its unoccupied state, freeing any previous
   * allocation requests.  This is also required for initializing the Arena
   * after construction.
   */
  void reset();

  /*
   * Allocates a chunk of at least the given number of bytes.  ("At least"
   * because the Arena tries to maintain 4-byte alignment and may round sizes
   * up.)
   *
   * If the allocation cannot be satisfied, asserts.  This behavior is probably
   * a tad extreme, but it's what m4vgalib is expecting; can revisit later.
   */
  void * allocate(std::size_t);

  std::size_t get_free_count() const { return _free_bytes; }
  std::size_t get_total_count() const { return _total_bytes; }

private:
  etl::data::RangePtr<Region const> _prototype;
  etl::data::RangePtr<Region> _table;
  std::size_t _total_bytes;
  std::size_t _free_bytes;
};

}  // namespace mem
}  // namespace etl

#endif  // _ETL_MEM_ARENA_H_INCLUDED
