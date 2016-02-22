#ifndef _ETL_STM32L4XX_APB_H_INCLUDED
#define _ETL_STM32L4XX_APB_H_INCLUDED

#include "etl/stm32l4xx/types.h"

namespace etl {
namespace stm32l4xx {

enum class ApbPeripheral : HalfWord {
  #define ETL_STM32L4XX_APB_P(name, bus, n, rst, enr, smenr) \
    name = n \
         | ((bus - 1) << 6) \
         | (rst << 7) \
         | (enr << 8) \
         | (smenr << 9),
  #include "etl/stm32l4xx/apb_peripherals.def"
  #undef ETL_STM32L4XX_APB_P
};

/*
 * Gets the bus index associated with an ApbPeripheral.  The result is
 * zero-based: APB1 = 0, APB2 = 1.
 */
inline unsigned get_bus_index(ApbPeripheral p) {
  return (static_cast<Word>(p) >> 6) & 1;
}

/*
 * Gets the extended bus index associated with an ApbPeripheral.  Used
 * internally, this has no relation to anything presented in the reference
 * manual.
 *
 * APB1 peripherals 0-31 = 0
 * APB1 peripherals 63-32 = 1
 * APB2 peripherals 0-31 = 2
 */
inline unsigned get_extended_bus_index(ApbPeripheral p) {
  return (static_cast<Word>(p) >> 5) & 2;
}

/*
 * Gets the bit index mapped to this peripheral in the RCC's control
 * registers.
 */
inline unsigned get_bit_index(ApbPeripheral p) {
  return static_cast<Word>(p) & 0x1F;
}

inline bool has_reset_control(ApbPeripheral p) {
  return static_cast<Word>(p) & (1 << 7);
}

inline bool has_clock_control(ApbPeripheral p) {
  return static_cast<Word>(p) & (1 << 8);
}

inline bool has_sleep_clock_control(ApbPeripheral p) {
  return static_cast<Word>(p) & (1 << 9);
}

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_APB_H_INCLUDED
