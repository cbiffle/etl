#ifndef _ETL_STM32F4XX_BASIC_TIMER_H_INCLUDED
#define _ETL_STM32F4XX_BASIC_TIMER_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct BasicTimer {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/basic_timer.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern BasicTimer tim6;
extern BasicTimer tim7;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_BASIC_TIMER_H_INCLUDED
