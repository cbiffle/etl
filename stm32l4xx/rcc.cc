#include "etl/stm32l4xx/rcc.h"

#include "etl/assert.h"
#include "etl/armv7m/instructions.h"

using etl::armv7m::data_synchronization_barrier;

namespace etl {
namespace stm32l4xx {

/*
 * AHB
 */

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

/*
 * APB
 */

void Rcc::enter_reset(ApbPeripheral p) {
  ETL_ASSERT(has_reset_control(p));

  unsigned bus = get_extended_bus_index(p);
  ETL_ASSERT(bus <= 2);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_apb1rstr1(read_apb1rstr1().with_bit(slot, true)); break;
    case 1: write_apb1rstr2(read_apb1rstr2().with_bit(slot, true)); break;
    case 2: write_apb2rstr(read_apb2rstr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::leave_reset(ApbPeripheral p) {
  ETL_ASSERT(has_reset_control(p));

  unsigned bus = get_bus_index(p);
  ETL_ASSERT(bus <= 2);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_apb1rstr1(read_apb1rstr1().with_bit(slot, false)); break;
    case 1: write_apb1rstr2(read_apb1rstr2().with_bit(slot, false)); break;
    case 2: write_apb2rstr(read_apb2rstr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

void Rcc::enable_clock(ApbPeripheral p) {
  ETL_ASSERT(has_clock_control(p));

  unsigned bus = get_bus_index(p);
  ETL_ASSERT(bus <= 2);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_apb1enr1(read_apb1enr1().with_bit(slot, true)); break;
    case 1: write_apb1enr2(read_apb1enr2().with_bit(slot, true)); break;
    case 2: write_apb2enr(read_apb2enr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::disable_clock(ApbPeripheral p) {
  ETL_ASSERT(has_clock_control(p));

  unsigned bus = get_bus_index(p);
  ETL_ASSERT(bus <= 2);
  unsigned slot = get_bit_index(p);

  switch (bus) {
    case 0: write_apb1enr1(read_apb1enr1().with_bit(slot, false)); break;
    case 1: write_apb1enr2(read_apb1enr2().with_bit(slot, false)); break;
    case 2: write_apb2enr(read_apb2enr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

}  // namespace stm32l4xx
}  // namespace etl
