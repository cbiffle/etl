#ifndef _ETL_ARMV7M_NVIC_H_INCLUDED
#define _ETL_ARMV7M_NVIC_H_INCLUDED

#include "etl/armv7m/types.h"

namespace etl {
namespace armv7m {

/*
 * The ARMv7-M Nested Vected Interrupt Controller, or NVIC, is defined by
 * section B3.4 of the ARMV7-M ARM.
 *
 * Recall that this architecture has both "exceptions" and "interrupts."  They
 * behave identically, but are configured separately.  The NVIC provides the
 * configuration and control interface for interrupts only.
 *
 * From the NVIC's perspective, there are up to 496 interrupts, numbered from
 * zero.  Most vendors implement fewer than this, and if you attempt to control
 * an interrupt that's out of range for your SoC, you're likely to find unmapped
 * memory and take a fault.  You have been warned.  The SoC layer will provide
 * a narrower interface that's harder to misuse; the direct NVIC interface is
 * primarily intended for use by the SoC layer.
 *
 * While, as per usual, all the NVIC's registers are available for your
 * inspection below, consider using the manipulator functions defined at the
 * end of this file.  They will help you avoid interesting race conditions.
 */
struct Nvic {
  #define ETL_BFF_DEFINITION_FILE "etl/armv7m/nvic.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE

 public:
  /*
   * Ensures that an interrupt is enabled.  If it was already enabled, there
   * is no change.
   *
   * The change happens atomically, both in terms of the memory write, and in
   * terms of the visible side effect:
   *  - The write is atomic with respect to other writes.
   *  - The interrupt becomes enabled before the function returns.  If the
   *    interrupt was pending and priority allows, the handler will run before
   *    control returns to the caller.
   *
   * You probably don't want to use this directly; the SoC layer should provide
   * a facade that takes a strongly typed interrupt enumeration.
   *
   * Precondition: IRQ number is valid for the current system.
   */
  void enable_irq(unsigned);

  /*
   * Ensures that an interrupt is disabled.  If it was already disabled, there
   * is no change.
   *
   * The change happens atomically, both in terms of the memory write, and in
   * terms of the visible side effect:
   *  - The write is atomic with respect to other writes.
   *  - The interrupt becomes disabled before the function returns.  The
   *    interrupt is guaranteed not to preempt once control returns to the
   *    caller.
   *
   * You probably don't want to use this directly; the SoC layer should provide
   * a facade that takes a strongly typed interrupt enumeration.
   *
   * Precondition: IRQ number is valid for the current system.
   */
  void disable_irq(unsigned);

  /*
   * Ensures that an interrupt is not pending.
   *
   * The change happens atomically, both in terms of the memory write, and in
   * terms of the visible side effect:
   *  - The write is atomic with respect to other writes.
   *  - The interrupt is not pending when the function returns.  If hardware
   *    generates another interrupt request, it may become pending again.
   *
   * You probably don't want to use this directly; the SoC layer should provide
   * a facade that takes a strongly typed interrupt enumeration.
   *
   * Precondition: IRQ number is valid for the current system.
   */
  void clear_pending_irq(unsigned);

  /*
   * Sets the priority for an interrupt source.
   *
   * The change happens atomically, both in terms of the memory write, and in
   * terms of the visible side effect:
   *  - The write uses exclusive store and will retry if it races another
   *    write.
   *  - The interrupt is not pending when the function returns.  If hardware
   *    generates another interrupt request, it may become pending again.
   *
   * Because of the retry strategy used to achieve write atomicity, there is
   * a small chance of infinite loop in systems with very high interrupt
   * frequency.  Applications may wish to wrap this operation in a critical
   * section if this is a concern.
   *
   * You probably don't want to use this directly; the SoC layer should provide
   * a facade that takes a strongly typed interrupt enumeration.
   *
   * Precondition: IRQ number is valid for the current system.
   */
  void set_irq_priority(unsigned, Byte);
};

extern Nvic nvic;

}  // namespace armv7m
}  // namespace etl

#endif  // _ETL_ARMV7M_NVIC_H_INCLUDED
