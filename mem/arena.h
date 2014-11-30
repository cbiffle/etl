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
 *
 * The arena's behavior in corner cases can be controlled by adjusting the
 * policy template parameters.  By default the arena asserts on misuse, but
 * this can be loosened for performance or to meet application requirements.
 */

#include <cstddef>
#include <cstdint>

#include "etl/assert.h"
#include "etl/data/range_ptr.h"

namespace etl {
namespace mem {

/*
 * Forward declarations of the policy defaults; see the bottom of this file for
 * details.
 */
struct AssertOnAllocationFailure;
struct DoNotRequireDeallocation;

/*
 * A region-based memory allocator.
 */
template <
  typename FailurePolicy = AssertOnAllocationFailure,
  typename TrackingPolicy = DoNotRequireDeallocation
>
class Arena : public TrackingPolicy {
public:
  /*
   * Creates an Arena using the given memory region.  The Arena is created in an
   * exhausted state and will deny allocation requests until it has been
   * reset().
   */
  Arena(etl::data::RangePtr<std::uint8_t> range)
    : _total(range),
      _free() {}

  /*
   * Restores this Arena to its unoccupied state, freeing any previous
   * allocation requests.  This is also required for initializing the Arena
   * after construction.
   */
  void reset() {
    TrackingPolicy::check_reset();

    _free = _total;
  }

  /*
   * Allocates a chunk of at least the given number of bytes.  ("At least"
   * because the Arena tries to maintain 4-byte alignment and may round sizes
   * up.)
   *
   * The behavior on allocation failure is determined by the failure policy.
   * If this function returns on allocation failure, it will return nullptr.
   */
  void * allocate(std::size_t bytes) {
    bytes = (bytes + 3) & ~3u;
  
    // Take space from the first region that can satisfy the request.
    if (_free.byte_length() >= bytes) {
      void * p = _free.base();
      _free = _free.tail_from(bytes);
      TrackingPolicy::note_allocation(p, bytes);
      return p;
    } else {
      return FailurePolicy::allocation_failed();
    }
  }

  std::size_t get_free_count() const { return _free.byte_length(); }
  std::size_t get_total_count() const { return _total.byte_length(); }

private:
  etl::data::RangePtr<std::uint8_t> _total;
  etl::data::RangePtr<std::uint8_t> _free;
};


/*
 * Arena failure policy that asserts when an allocation fails, rather than
 * exposing the caller to a nullptr.
 */
struct AssertOnAllocationFailure {
  static void * allocation_failed() {
    ETL_ASSERT(false);
  }
};

/*
 * Arena failure policy that returns nullptr when an allocation fails.
 */
struct ReturnNullptrOnAllocationFailure {
  static void * allocation_failed() {
    return nullptr;
  }
};


/*
 * Arena tracking policy that does not require deallocation of allocated space,
 * and allows reset() at any time.  This is more efficient (in both time and
 * space) but allows for accidental Arena misuse, particularly in the presence
 * of types with non-trivial destructors.
 */
struct DoNotRequireDeallocation {
  static void check_reset() {
    // Good to go.
  }

  static void note_allocation(void *, std::size_t) {
    // Don't care.
  }
};

/*
 * Arena tracking policy that exposes a deallocate operation, and requires a
 * deallocate call for every allocate.  This provides some insurance against
 * misuse of the Arena, though note that "deallocating" here does not make the
 * memory available for re-allocation until reset().
 *
 * TODO(cbiffle): this can't currently verify that the pointer given to
 * deallocate even belongs to this Arena, much less that it's one we previously
 * handed out.  We could potentially allocate an additional word and store a
 * random brand there for probabilistic verification....
 */
class RequireMatchingDeallocation {
public:
  /*
   * Signals to the Arena that a pointer previously returned by allocate() is
   * no longer in use.
   */
  void deallocate(void * ptr) {
    ETL_ASSERT(_allocation_count);
    --_allocation_count;
  }

  /*
   * Retrieves the number of outstanding (not yet deallocated) allocation
   * requests.
   */
  std::size_t get_allocation_count() const { return _allocation_count; }

protected:
  RequireMatchingDeallocation() : _allocation_count(0) {}

  void note_allocation(void *, std::size_t) {
    ++_allocation_count;
  }

  void check_reset() {
    ETL_ASSERT(_allocation_count == 0);
  }

private:
  std::size_t _allocation_count;
};

}  // namespace mem
}  // namespace etl

#endif  // _ETL_MEM_ARENA_H_INCLUDED
