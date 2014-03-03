#ifndef ETL_STM32F4XX_INTERRUPT_TABLE_H_
#define ETL_STM32F4XX_INTERRUPT_TABLE_H_

extern "C" {
  #define ETL_STM32F4XX_IRQ(name) void etl_stm32f4xx_ ## name ## _handler();
  #include "etl/stm32f4xx/interrupts.def"
  #undef ETL_STM32F4XX_IRQ
}

#endif  // ETL_STM32F4XX_INTERRUPT_TABLE_H_
