#include "etl/armv7m/nvic.h"
#include "etl/armv7m/instructions.h"

namespace etl {
namespace armv7m {

void Nvic::enable_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  write_iser(bank, iser_value_t().with_bit(index, true));
  data_memory_barrier();
  instruction_synchronization_barrier();
}

void Nvic::disable_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  write_icer(bank, icer_value_t().with_bit(index, true));
  data_memory_barrier();
  instruction_synchronization_barrier();
}

void Nvic::clear_pending_irq(unsigned irq) {
  unsigned bank = irq / 32;
  unsigned index = irq % 32;

  write_icpr(bank, icpr_value_t().with_bit(index, true));
  data_memory_barrier();
  instruction_synchronization_barrier();
}

void Nvic::set_irq_priority(unsigned irq, Byte priority) {
  unsigned bank = irq / 4;
  unsigned index = irq % 4;

  update_ipr(bank, [&](ipr_value_t x) {
      return x.with_priority(index, priority);
  });

  // swap implies a DMB.
  instruction_synchronization_barrier();
}

}  // namespace armv7m
}  // namespace etl
