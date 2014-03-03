#include "etl/stm32f4xx/interrupts.h"

#include "etl/armv7m/nvic.h"

#include "etl/stm32f4xx/types.h"

using etl::armv7m::nvic;

namespace etl {
namespace stm32f4xx {

void enable_irq(Interrupt irq) {
  nvic.enable_irq(static_cast<unsigned>(irq));
}

void disable_irq(Interrupt irq) {
  nvic.disable_irq(static_cast<unsigned>(irq));
}

void clear_pending_irq(Interrupt irq) {
  nvic.clear_pending_irq(static_cast<unsigned>(irq));
}

void set_irq_priority(Interrupt irq, unsigned priority) {
  unsigned shifted_priority = (priority << 4) & 0xFF;
  nvic.set_irq_priority(static_cast<unsigned>(irq),
                        static_cast<Byte>(shifted_priority));
}

}  // namespace stm32f4xx
}  // namespace etl
