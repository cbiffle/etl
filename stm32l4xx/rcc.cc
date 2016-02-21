#include "etl/stm32l4xx/rcc.h"

#include "etl/assert.h"
#include "etl/armv7m/instructions.h"

using etl::armv7m::data_synchronization_barrier;

namespace etl {
namespace stm32l4xx {

void Rcc::enter_reset(AhbPeripheral p) {
  ETL_ASSERT(has_reset_control(p));

  unsigned bus = get_bus_index(p);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, true)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, true)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::leave_reset(AhbPeripheral p) {
  ETL_ASSERT(has_reset_control(p));

  unsigned bus = get_bus_index(p);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, false)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, false)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

void Rcc::enable_clock(AhbPeripheral p) {
  ETL_ASSERT(has_clock_control(p));

  unsigned bus = get_bus_index(p);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, true)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, true)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::disable_clock(AhbPeripheral p) {
  ETL_ASSERT(has_clock_control(p));

  unsigned bus = get_bus_index(p);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, false)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, false)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

}  // namespace stm32l4xx
}  // namespace etl
