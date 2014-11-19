#ifndef _ETL_STM32F4XX_IIC_H_INCLUDED
#define _ETL_STM32F4XX_IIC_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Iic {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/iic.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Iic iic1;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_IIC_H_INCLUDED
