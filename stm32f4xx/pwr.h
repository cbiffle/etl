#ifndef _ETL_STM32F4XX_PWR_H_INCLUDED
#define _ETL_STM32F4XX_PWR_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Pwr {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/pwr.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Pwr pwr;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_PWR_H_INCLUDED
