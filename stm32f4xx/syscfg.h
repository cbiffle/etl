#ifndef _ETL_STM32F4XX_SYSCFG_H_INCLUDED
#define _ETL_STM32F4XX_SYSCFG_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct SysCfg {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/syscfg.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern SysCfg syscfg;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_SYSCFG_H_INCLUDED
