#ifndef _ETL_STM32F4XX_GPIO_H_INCLUDED
#define _ETL_STM32F4XX_GPIO_H_INCLUDED

#include "etl/attribute_macros.h"
#include "etl/stm32f4xx/types.h"

namespace etl {
namespace stm32f4xx {

struct Gpio {
  enum class Mode {
    input     = 0b00,
    gpio      = 0b01,
    alternate = 0b10,
    analog    = 0b11,
  };

  enum class OutputType {
    push_pull = 0,
    open_drain = 1,
  };

  enum class OutputSpeed {
    low_2mhz     = 0b00,
    medium_25mhz = 0b01,
    fast_50mhz   = 0b10,
    high_100mhz  = 0b11,
  };

  enum class Pull {
    none = 0b00,
    up   = 0b01,
    down = 0b10,
    // 0b11 is reserved
  };

  #define ETL_BFF_DEFINITION_FILE "etl/stm32f4xx/gpio.reg"
  #include "etl/biffield/generate.h"
  #undef ETL_BFF_DEFINITION_FILE

  /*
   * Changes the mode of each pin whose corresponding bit in the mask is 1.
   */
  void set_mode(HalfWord mask, Mode);

  /*
   * Changes the output type of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_output_type(HalfWord mask, OutputType);

  /*
   * Changes the output speed of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_output_speed(HalfWord mask, OutputSpeed);

  /*
   * Changes the pull of each pin whose corresponding bit in the mask is
   * 1.
   */
  void set_pull(HalfWord mask, Pull);

  /*
   * Changes the alternate function of each pin whose corresponding bit in the
   * mask is 1.  Note that this is not atomic across the port, unlike the other
   * mask mode setters above.
   */
  void set_alternate_function(HalfWord mask, unsigned);

  /*
   * Sets the output data latch for any pins whose corresponding bit in the
   * mask is 1.  This will become apparent the next time the pin is an output.
   */
  ETL_INLINE void set(HalfWord mask) {
    write_bsrr(bsrr_value_t().with_setbits(mask));
  }

  /*
   * Clears the output data latch for any pins whose corresponding bit in the
   * mask is 1.  This will become apparent the next time the pin is an output.
   */
  ETL_INLINE void clear(HalfWord mask) {
    write_bsrr(bsrr_value_t().with_resetbits(mask));
  }

  /*
   * Toggles every pin whose corresponding bit in the mask is 1.
   */
  ETL_INLINE void toggle(HalfWord mask) {
    auto bits = read_odr().get_bits();
    write_bsrr(bsrr_value_t()
               .with_setbits(HalfWord(~bits & mask))
               .with_resetbits(HalfWord(bits & mask)));
  }


  enum Mask {
    #define ETL_STM32F4XX_GPIO_MASK(n) p ## n = 1 << n,
    ETL_STM32F4XX_GPIO_MASK(0)
    ETL_STM32F4XX_GPIO_MASK(1)
    ETL_STM32F4XX_GPIO_MASK(2)
    ETL_STM32F4XX_GPIO_MASK(3)
    ETL_STM32F4XX_GPIO_MASK(4)
    ETL_STM32F4XX_GPIO_MASK(5)
    ETL_STM32F4XX_GPIO_MASK(6)
    ETL_STM32F4XX_GPIO_MASK(7)

    ETL_STM32F4XX_GPIO_MASK(8)
    ETL_STM32F4XX_GPIO_MASK(9)
    ETL_STM32F4XX_GPIO_MASK(10)
    ETL_STM32F4XX_GPIO_MASK(11)
    ETL_STM32F4XX_GPIO_MASK(12)
    ETL_STM32F4XX_GPIO_MASK(13)
    ETL_STM32F4XX_GPIO_MASK(14)
    ETL_STM32F4XX_GPIO_MASK(15)
    #undef ETL_STM32F4XX_GPIO_MASK
  };
};

extern Gpio gpioa;
extern Gpio gpiob;
extern Gpio gpioc;
extern Gpio gpiod;
extern Gpio gpioe;
extern Gpio gpiof;
extern Gpio gpiog;
extern Gpio gpioh;
extern Gpio gpioi;
extern Gpio gpioj;
extern Gpio gpiok;

}  // namespace stm32f4xx
}  // namespace etl

#endif  // _ETL_STM32F4XX_GPIO_H_INCLUDED
