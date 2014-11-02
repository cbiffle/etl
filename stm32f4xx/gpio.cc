#include "etl/stm32f4xx/gpio.h"

namespace etl {
namespace stm32f4xx {

/*
 * Each of the GPIO registers gets a very similar mutator.  Generate these
 * with a macro to avoid copy-pasting.
 */

#define GPIO_MUTATOR(name, type, reg, field) \
  void Gpio::set_ ## name (HalfWord mask_, type x) { \
    auto val = read_ ## reg(); \
    Word mask = mask_; \
    for (unsigned i = 0; i < 16; ++i) { \
      if (mask & 1) val = val.with_ ## field(i, x); \
      mask >>= 1; \
    } \
    write_ ## reg(val); \
  }

GPIO_MUTATOR(mode, Mode, moder, mode)
GPIO_MUTATOR(output_type, OutputType, otyper, otype)
GPIO_MUTATOR(output_speed, OutputSpeed, ospeedr, ospeed)
GPIO_MUTATOR(pull, Pull, pupdr, pupd)

void Gpio::set_alternate_function(HalfWord mask_, unsigned af) {
  af &= 0xF;

  auto val_low = read_afrl();
  auto val_high = read_afrh();

  Word mask = mask_;

  for (unsigned i = 0; i < 8; ++i) {
    if (mask & 1) val_low = val_low.with_af(i, af);
    if (mask & (1 << 8)) val_high = val_high.with_af(i, af);
    mask >>= 1;
  }

  write_afrl(val_low);
  write_afrh(val_high);
}

}  // namespace stm32f4xx
}  // namespace etl
