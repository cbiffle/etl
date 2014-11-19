#ifndef _ETL_STM32F4XX_SPI_H_INCLUDED
#define _ETL_STM32F4XX_SPI_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Spi {
  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/spi.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Spi spi3;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_SPI_H_INCLUDED
