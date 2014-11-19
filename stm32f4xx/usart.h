#ifndef _ETL_STM32F4XX_USART_H_INCLUDED
#define _ETL_STM32F4XX_USART_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Usart {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/usart.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Usart usart1;
extern Usart usart2;
extern Usart usart3;
extern Usart usart6;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_USART_H_INCLUDED
