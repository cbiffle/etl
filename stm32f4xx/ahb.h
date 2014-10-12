#ifndef _ETL_STM32F4XX_AHB_H_INCLUDED
#define _ETL_STM32F4XX_AHB_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

/*
 * Names for AHB peripherals, so that we can reset/enable them generically.
 * The enum values *happen* to contain some useful information, but you should
 * not rely on this unless you're the RCC driver -- it may change.
 */
enum class AhbPeripheral : Word {
  /*
   * AHB1 peripherals
   */
  #define ETL_NONE (0xFF)
  #define ETL_STM32F4XX_AHB_PERIPH(name, rst, en) \
    name = (0 << 16) | (rst << 8) | (en << 0),
  #include "etl/stm32f4xx/ahb1_peripherals.def"
  #undef ETL_STM32F4XX_AHB_PERIPH
  #undef ETL_NONE
};

inline unsigned get_bus_index(AhbPeripheral p) {
  return static_cast<Byte>(static_cast<Word>(p) >> 16);
}

inline unsigned get_reset_index(AhbPeripheral p) {
  return static_cast<Byte>(static_cast<Word>(p) >> 8);
}

inline unsigned get_enable_index(AhbPeripheral p) {
  return static_cast<Byte>(p);
}

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_AHB_H_INCLUDED
