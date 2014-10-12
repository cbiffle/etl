#ifndef _ETL_STM32F4XX_APB_H_INCLUDED
#define _ETL_STM32F4XX_APB_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

enum class ApbPeripheral : HalfWord {
  /*
   * APB1 peripherals
   */
  #define ETL_STM32F4XX_APB_PERIPH(slot, name) name = (0 << 8) | slot,
  #define ETL_STM32F4XX_APB_RESERVED(slot) /* nothing */
  #include "etl/stm32f4xx/apb1_peripherals.def"
  #undef ETL_STM32F4XX_APB_RESERVED
  #undef ETL_STM32F4XX_APB_PERIPH

  /*
   * APB2 peripherals
   */
  #define ETL_STM32F4XX_APB_PERIPH(slot, name) name = (1 << 8) | slot,
  #define ETL_STM32F4XX_APB_RESERVED(slot) /* nothing */
  #include "etl/stm32f4xx/apb2_peripherals.def"
  #undef ETL_STM32F4XX_APB_RESERVED
  #undef ETL_STM32F4XX_APB_PERIPH
};

inline unsigned get_bus_index(ApbPeripheral p) {
  return (static_cast<Word>(p) >> 8) & 0xFF;
}

inline unsigned get_slot_index(ApbPeripheral p) {
  return static_cast<Word>(p) & 0xFF;
}

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_APB_H_INCLUDED
