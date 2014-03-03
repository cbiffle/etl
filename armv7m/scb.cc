#include "etl/armv7m/scb.h"

#include "etl/armv7m/instructions.h"

namespace etl {
namespace armv7m {

void Scb::enable_faults() {
  bool succeeded = false;
  do {
    auto before = read_shcsr();
    succeeded = swap_shcsr(before, before.with_memfaultena(true)
                                         .with_busfaultena(true)
                                         .with_usgfaultena(true));
  } while (!succeeded);
  instruction_synchronization_barrier();
}

void Scb::set_exception_priority(Exception e, Byte p) {
  unsigned index = static_cast<unsigned>(e);
  if (index < 4 || index > 15) {
    // Not configurable.
    // TODO(cbiffle): assert.
    return;
  }

  unsigned bank = (index - 4) / 4;
  unsigned slot = index % 4;

  switch (bank) {
    case 0: write_shpr1(read_shpr1().with_pri(slot, p)); break;
    case 1: write_shpr2(read_shpr2().with_pri(slot, p)); break;
    case 2: write_shpr3(read_shpr3().with_pri(slot, p)); break;
  }
}

}  // namespace armv7m
}  // namespace etl
