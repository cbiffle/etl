#include "etl/stm32l4xx/interrupt_table.h"
#include "etl/attribute_macros.h"
#include "etl/stm32l4xx/interrupts.h"

namespace etl {
namespace stm32l4xx {


typedef void (*InterruptHandler)(void);

struct InterruptTable {
  #define ETL_STM32L4XX_IRQ(name) InterruptHandler name ## _handler;
  #define ETL_STM32L4XX_IRQ_RESERVED(n) InterruptHandler _reserved_##n;
  #include "etl/stm32l4xx/interrupts.def"
  #undef ETL_STM32L4XX_IRQ
  #undef ETL_STM32L4XX_IRQ_RESERVED
};

/*
 * There are a couple of nuances to the interrupt table definition.
 *
 *  1. We place it in a section called .etl_stm32l4xx_interrupt_table.  There's
 *     nothing special about that name, except that the linker scripts look for
 *     it and put it in the right place.
 *
 *  2. We make the table const, ensuring that the linker will let us place it
 *     in Flash if the linker script wants it there.
 */
ETL_SECTION(".etl_stm32l4xx_interrupt_table")
ETL_USED
InterruptTable const stm32l4xx_interrupt_table = {
  #define ETL_STM32L4XX_IRQ(name) etl_stm32l4xx_ ## name ## _handler,
  #define ETL_STM32L4XX_IRQ_RESERVED(_) nullptr,
  #include "etl/stm32l4xx/interrupts.def"
  #undef ETL_STM32L4XX_IRQ
  #undef ETL_STM32L4XX_IRQ_RESERVED
};

extern "C" {
  void unexpected_irq();  // hack
  void unexpected_irq() { while (1); }
  #define ETL_STM32L4XX_IRQ(name) void etl_stm32l4xx_ ## name ## _handler() \
                                  ETL_WEAK ETL_ALIAS("unexpected_irq");
  #define ETL_STM32L4XX_IRQ_RESERVED(_) /* nothing to see here */
  #include "etl/stm32l4xx/interrupts.def"
  #undef ETL_STM32L4XX_IRQ
  #undef ETL_STM32L4XX_IRQ_RESERVED
}

}  // namespace stm32l4xx
}  // namespace etl
