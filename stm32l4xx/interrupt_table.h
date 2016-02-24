#ifndef _ETL_STM32L4XX_INTERRUPT_TABLE_H_INCLUDED
#define _ETL_STM32L4XX_INTERRUPT_TABLE_H_INCLUDED

extern "C" {
  #define ETL_STM32L4XX_IRQ(name) void etl_stm32l4xx_ ## name ## _handler();
  #define ETL_STM32L4XX_IRQ_RESERVED(_) /* nothing to see here */
  #include "etl/stm32l4xx/interrupts.def"
  #undef ETL_STM32L4XX_IRQ
  #undef ETL_STM32L4XX_IRQ_RESERVED
}

#endif  // _ETL_STM32L4XX_INTERRUPT_TABLE_H_INCLUDED
