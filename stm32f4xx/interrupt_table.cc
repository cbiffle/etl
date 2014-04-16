#include "etl/stm32f4xx/interrupt_table.h"
#include "etl/attribute_macros.h"
#include "etl/stm32f4xx/interrupts.h"

namespace etl {
namespace stm32f4xx {


typedef void (*InterruptHandler)(void);

struct InterruptTable {
  #define ETL_STM32F4XX_IRQ(name) InterruptHandler name ## _handler;
  #include "etl/stm32f4xx/interrupts.def"
  #undef ETL_STM32F4XX_IRQ
};

/*
 * There are a couple of nuances to the interrupt table definition.
 *
 *  1. We place it in a section called .etl_stm32f4xx_interrupt_table.  There's
 *     nothing special about that name, except that the linker scripts look for
 *     it and put it in the right place.
 *
 *  2. We make the table const, ensuring that the linker will let us place it
 *     in Flash if the linker script wants it there.
 */
ETL_SECTION(".etl_stm32f4xx_interrupt_table")
ETL_USED
InterruptTable const stm32f4xx_interrupt_table = {
  #define ETL_STM32F4XX_IRQ(name) etl_stm32f4xx_ ## name ## _handler,
  #include "etl/stm32f4xx/interrupts.def"
  #undef ETL_STM32F4XX_IRQ
};

extern "C" {
  void unexpected_irq();  // hack
  void unexpected_irq() { while (1); }
  #define ETL_STM32F4XX_IRQ(name) void etl_stm32f4xx_ ## name ## _handler() \
                                  ETL_WEAK ETL_ALIAS("unexpected_irq");
  #include "etl/stm32f4xx/interrupts.def"
  #undef ETL_STM32F4XX_IRQ
}

}  // namespace stm32f4xx
}  // namespace etl
