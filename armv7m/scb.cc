#include "etl/armv7m/scb.h"

#include "etl/assert.h"
#include "etl/armv7m/instructions.h"

namespace etl {
namespace armv7m {

void Scb::enable_faults() {
  update_shcsr([] (shcsr_value_t v) {
      return v.with_memfaultena(true)
              .with_busfaultena(true)
              .with_usgfaultena(true);
  });
  instruction_synchronization_barrier();
}

void Scb::set_exception_priority(Exception e, Byte p) {
  unsigned index = static_cast<unsigned>(e);
  ETL_ASSERT(index >= 4 && index <= 15);

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
