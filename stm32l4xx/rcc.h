#ifndef _ETL_STM32L4XX_RCC_H_INCLUDED
#define _ETL_STM32L4XX_RCC_H_INCLUDED

#include "etl/stm32l4xx/types.h"
#include "etl/stm32l4xx/ahb.h"
#include "etl/stm32l4xx/apb.h"

namespace etl {
namespace stm32l4xx {

struct ClockConfig;

struct Rcc {
  enum class pprex_t : unsigned {
    div1 = 0b000,
    div2 = 0b100,
    div4 = 0b101,
    div8 = 0b110,
    div16 = 0b111,
  };

  #define ETL_BFF_DEFINITION_FILE "etl/stm32l4xx/rcc.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE

  void enter_reset(AhbPeripheral);
  void enter_reset(ApbPeripheral);

  void leave_reset(AhbPeripheral);
  void leave_reset(ApbPeripheral);

  void enable_clock(AhbPeripheral);
  void enable_clock(ApbPeripheral);

  void disable_clock(AhbPeripheral);
  void disable_clock(ApbPeripheral);
};

extern Rcc rcc;

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_RCC_H_INCLUDED
