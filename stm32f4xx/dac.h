#ifndef _ETL_STM32F4XX_DAC_H_INCLUDED
#define _ETL_STM32F4XX_DAC_H_INCLUDED

#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Dac {
  enum class LfsrMask : unsigned {
    unmask_1_lsb = 0b0000,
    unmask_2_lsbs = 0b0001,
    unmask_3_lsbs = 0b0010,
    unmask_4_lsbs = 0b0011,
    unmask_5_lsbs = 0b0100,
    unmask_6_lsbs = 0b0101,
    unmask_7_lsbs = 0b0110,
    unmask_8_lsbs = 0b0111,
    unmask_9_lsbs = 0b1000,
    unmask_10_lsbs = 0b1001,
    unmask_11_lsbs = 0b1010,
    unmask_12_lsbs = 0b1011,
  };

  enum class TriangleAmplitude : unsigned {
    amplitude_1 = 0b0000,
    amplitude_3 = 0b0001,
    amplitude_7 = 0b0010,
    amplitude_15 = 0b0011,
    amplitude_31 = 0b0100,
    amplitude_63 = 0b0101,
    amplitude_127 = 0b0110,
    amplitude_255 = 0b0111,
    amplitude_511 = 0b1000,
    amplitude_1023 = 0b1001,
    amplitude_2047 = 0b1010,
    amplitude_4095 = 0b1011,
  };

  enum class Waveform : unsigned {
    disabled = 0b00,
    noise = 0b01,
    triangle = 0b10,
  };

  enum class Trigger : unsigned {
    tim6_trgo = 0b000,
    tim8_trgo = 0b001,
    tim7_trgo = 0b010,
    tim5_trgo = 0b011,
    tim2_trgo = 0b100,
    tim4_trgo = 0b101,
    exti9 = 0b110,
    swtrig = 0b111,
  };

  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/dac.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE
};

extern Dac dac;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_DAC_H_INCLUDED
