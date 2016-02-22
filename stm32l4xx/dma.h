#ifndef _ETL_STM32L4XX_DMA_H_INCLUDED
#define _ETL_STM32L4XX_DMA_H_INCLUDED

#include "etl/stm32l4xx/types.h"

namespace etl {
namespace stm32l4xx {

struct Dma {
  struct Channel {
    enum class TransferSize {
      byte = 0b00,
      half_word = 0b01,
      word = 0b10,
    };

    #define ETL_BFF_DEFINITION_FILE "etl/stm32l4xx/dma_channel.reg"
    #include "etl/biffield/generate.h"
    #undef ETL_BFF_DEFINITION_FILE
  };

  #define ETL_BFF_DEFINITION_FILE "etl/stm32l4xx/dma_interrupts.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE

  Channel ch1;
  Channel ch2;
  Channel ch3;
  Channel ch4;
  Channel ch5;
  Channel ch6;
  Channel ch7;

  #define ETL_BFF_DEFINITION_FILE "etl/stm32l4xx/dma_channel_selection.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Dma dma1;
extern Dma dma2;

}  // namespace stm32l4xx
}  // namespace etl

#endif  // _ETL_STM32L4XX_DMA_H_INCLUDED
