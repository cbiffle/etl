#ifndef _ETL_STM32F4XX_INTERRUPTS_H_INCLUDED
#define _ETL_STM32F4XX_INTERRUPTS_H_INCLUDED

namespace etl {
namespace stm32f4xx {

/*
 * A convenient enumeration of all STM32F4xx interrupts.
 */
enum class Interrupt : unsigned {
  #define ETL_STM32F4XX_IRQ(name) name,
  #include "etl/stm32f4xx/interrupts.def"
  #undef ETL_STM32F4XX_IRQ
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
 * Facade for armv7m::nvic.disable_irq using the Interrupt enum *and* STM32F4xx
 * priorities.
 *
 * The STM32F4xx only implements four bits of priority, so valid priorities
 * range from 0 - 15.
 *
 * Precondition: priority is valid.
 */
void set_irq_priority(Interrupt, unsigned priority);

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_INTERRUPTS_H_INCLUDED
