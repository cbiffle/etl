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

}  // namespace armv7m
}  // namespace etl
