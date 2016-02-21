#ifndef _ETL_STM32L4XX_AHB_H_INCLUDED
#define _ETL_STM32L4XX_AHB_H_INCLUDED

#include "etl/stm32l4xx/types.h"

namespace etl {
namespace stm32l4xx {

/*
 * Names for AHB peripherals, so that we can reset/enable them generically.
 * The enum values *happen* to contain some useful information, but you should
 * not rely on this unless you're the RCC driver -- it may change.
 */
enum class AhbPeripheral : Word {
  /*
   * AHB1 peripherals
   */
  #define ETL_STM32L4XX_AHB_PERIPH(name, n, rst, enr, smenr) \
    name = (0 << 0) | (n << 4) | (rst << 9) | (enr << 10) | (smenr << 11),
  #include "etl/stm32l4xx/ahb1_peripherals.def"
  #undef ETL_STM32L4XX_AHB_PERIPH

  /*
   * AHB2 peripherals
   */
  #define ETL_STM32L4XX_AHB_PERIPH(name, n, rst, enr, smenr) \
    name = (1 << 0) | (n << 4) | (rst << 9) | (enr << 10) | (smenr << 11),
  #include "etl/stm32l4xx/ahb2_peripherals.def"
  #undef ETL_STM32L4XX_AHB_PERIPH

  /*
   * AHB3 peripherals
   */
  #define ETL_STM32L4XX_AHB_PERIPH(name, n, rst, enr, smenr) \
    name = (2 << 0) | (n << 4) | (rst << 9) | (enr << 10) | (smenr << 11),
  #include "etl/stm32l4xx/ahb3_peripherals.def"
  #undef ETL_STM32L4XX_AHB_PERIPH

};

inline unsigned get_bus_index(AhbPeripheral p) {
  return static_cast<Word>(p) & 0xF;
}

inline unsigned get_bit_index(AhbPeripheral p) {
  return (static_cast<Word>(p) >> 4) & 0x1F;
}

inline bool has_reset_control(AhbPeripheral p) {
  return static_cast<Word>(p) & (1 << 9);
}

inline bool has_clock_control(AhbPeripheral p) {
  return static_cast<Word>(p) & (1 << 10);
}

inline bool has_sleep_clock_control(AhbPeripheral p) {
  return static_cast<Word>(p) & (1 << 11);
}

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_AHB_H_INCLUDED
