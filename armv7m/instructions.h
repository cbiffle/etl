#ifndef _ETL_ARMV7M_INSTRUCTIONS_H_INCLUDED
#define _ETL_ARMV7M_INSTRUCTIONS_H_INCLUDED

/*
 * C++ function wrappers for GCC ARM intrinsics and inline assembly.
 */

#include <cstdint>

#include "etl/attribute_macros.h"

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
 * Disables interrupts ("cpsid i").  On ARMv7-M this uses PRIMASK.
 */
ETL_INLINE void disable_interrupts() {
  asm volatile ("cpsid i");
}

/*
 * Enables interrupts ("cpsie i").  On ARMv7-M this uses PRIMASK.
 */
ETL_INLINE void enable_interrupts() {
  asm volatile ("cpsie i");
}



}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_INSTRUCTIONS_H_INCLUDED
