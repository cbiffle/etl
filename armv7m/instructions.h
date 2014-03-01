#ifndef ETL_ARMV7M_INSTRUCTIONS_H_
#define ETL_ARMV7M_INSTRUCTIONS_H_

/*
 * C++ function wrappers for GCC ARM intrinsics and inline assembly.
 */

#include "etl/common/attribute_macros.h"

namespace etl {
namespace armv7m {

/*
 * Wait For Interrupt -- idles the processor until an interrupt arrives.
 * This can frequently save power, depending on how the processor is configured.
 */
ETL_INLINE void wait_for_interrupt() {
  asm volatile ("wfi");
}

/*
 * Data Synchronization Barrier -- ensures that all memory accesses that occur
 * before this point (in program order) have completed before the next
 * instruction executes.  This is useful when writing to system control
 * registers.
 *
 * Note that a DSB is *not* sufficient when writing a register that may affect
 * what areas of memory can be executed.  It doesn't prevent the processor from
 * fetching instructions past the DSB, just executing them.  For that, you also
 * need an ISB (below).
 */
ETL_INLINE void data_synchronization_barrier() {
  asm volatile ("dsb");
}

/*
 * Data Memory Barrier -- ensures that all memory accesses that occur before
 * this point (in program order) complete before any memory accesses that occur
 * after this point.
 */
ETL_INLINE void data_memory_barrier() {
  asm volatile ("dmb");
}

/*
 * Instruction Synchronization Barrier -- ensures that all instructions after
 * this point (in program order) are fetched from memory only after any context
 * changes before this point have completed.  In particular, this is necessary
 * when changing memory cache settings, access permissions, branch prediction,
 * or interrupt priorities.
 *
 * This is effectively equivalent to (and roughly as costly as) a complete
 * flush of all processor pipelines.
 *
 * Note that this doesn't necessarily imply a data barrier.  If your context
 * altering operation is a result of writing memory, you probably also need a
 * DSB (above).
 */
ETL_INLINE void instruction_synchronization_barrier() {
  asm volatile ("isb");
}

/*
 * Saturate an unsigned integer to a certain number of bit positions with an
 * optional shift.  Negative shift distance shift right.
 */
template <unsigned N, int S = 0>
ETL_INLINE unsigned usat(unsigned v) {
  register unsigned r;
  if (S >= 0) {
    static_assert(S < 0 || (S <= 31), "left shift out of range");
    asm ("usat %0, %1, %2, LSL %3"
         : "=r" (r)
         : "I" (N), "r" (v), "I" (S));
    return r;
  } else {
    static_assert(S >= 0 || (-S >= 1 && -S <= 31), "right shift out of range");
    asm ("usat %0, %1, %2, ASR %3"
         : "=r" (r)
         : "I" (N), "r" (v), "I" (-S));
    return r;
  }
}

}  // namespace armv7m
}  // namespace etl

#endif  // ETL_ARMV7M_INSTRUCTIONS_H_
