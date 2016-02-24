#ifndef _ETL_STM32L4XX_INTERRUPTS_H_INCLUDED
#define _ETL_STM32L4XX_INTERRUPTS_H_INCLUDED

namespace etl {
namespace stm32l4xx {

/*
 * A convenient enumeration of all STM32L4xx interrupts.
 */
enum class Interrupt : unsigned {
  #define ETL_STM32L4XX_IRQ(name) name,
  #define ETL_STM32L4XX_IRQ_RESERVED(offset) _reserved_##offset,
  #include "etl/stm32l4xx/interrupts.def"
  #undef ETL_STM32L4XX_IRQ
  #undef ETL_STM32L4XX_IRQ_RESERVED
};

/*
 * Facade for armv7m::nvic.enable_irq using the Interrupt enum.
 */
void enable_irq(Interrupt);

/*
 * Facade for armv7m::nvic.disable_irq using the Interrupt enum.
 */
void disable_irq(Interrupt);

/*
 * Facade for armv7m::nvic.clear_pending_irq using the Interrupt enum.
 */
void clear_pending_irq(Interrupt);

/*
 * Facade for armv7m::nvic.disable_irq using the Interrupt enum *and* STM32L4xx
 * priorities.
 *
 * The STM32L4xx only implements four bits of priority, so valid priorities
 * range from 0 - 15.
 *
 * Precondition: priority is valid.
 */
void set_irq_priority(Interrupt, unsigned priority);

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_INTERRUPTS_H_INCLUDED
