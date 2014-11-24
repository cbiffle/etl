#include "etl/stm32f4xx/rcc.h"

#include "etl/assert.h"
#include "etl/armv7m/instructions.h"
#include "etl/stm32f4xx/flash.h"

using etl::armv7m::data_synchronization_barrier;

namespace etl {
namespace stm32f4xx {

void Rcc::enter_reset(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1rstr(read_apb1rstr().with_bit(slot, true)); break;
    case 1: write_apb2rstr(read_apb2rstr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::leave_reset(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1rstr(read_apb1rstr().with_bit(slot, false)); break;
    case 1: write_apb2rstr(read_apb2rstr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

void Rcc::enable_clock(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1enr(read_apb1enr().with_bit(slot, true)); break;
    case 1: write_apb2enr(read_apb2enr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::disable_clock(ApbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_slot_index(p);

  switch (bus) {
    case 0: write_apb1enr(read_apb1enr().with_bit(slot, false)); break;
    case 1: write_apb2enr(read_apb2enr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}


void Rcc::enter_reset(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_reset_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, true)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, true)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::leave_reset(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_reset_index(p);

  switch (bus) {
    case 0: write_ahb1rstr(read_ahb1rstr().with_bit(slot, false)); break;
    case 1: write_ahb2rstr(read_ahb2rstr().with_bit(slot, false)); break;
    case 2: write_ahb3rstr(read_ahb3rstr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}

void Rcc::enable_clock(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_enable_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, true)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, true)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, true)); break;
  }
  data_synchronization_barrier();
}

void Rcc::disable_clock(AhbPeripheral p) {
  unsigned bus = get_bus_index(p);
  unsigned slot = get_enable_index(p);

  switch (bus) {
    case 0: write_ahb1enr(read_ahb1enr().with_bit(slot, false)); break;
    case 1: write_ahb2enr(read_ahb2enr().with_bit(slot, false)); break;
    case 2: write_ahb3enr(read_ahb3enr().with_bit(slot, false)); break;
  }
  data_synchronization_barrier();
}


static ClockSpeeds clock_speeds;

static Rcc::cfgr_value_t::hpre_t get_hpre(unsigned divisor) {
  switch (divisor) {
    #define X(x) case x: return Rcc::cfgr_value_t::hpre_t::div ## x;
    X(  1)
    X(  2)
    X(  4)
    X(  8)
    X( 16)
    X( 64)
    X(128)
    X(256)
    X(512)
    #undef X

    default:
      ETL_ASSERT(false);
  }
}

static Rcc::pprex_t get_ppre(unsigned divisor) {
  switch (divisor) {
    #define X(x) case x: return Rcc::pprex_t::div ## x;
    X(  1)
    X(  2)
    X(  4)
    X(  8)
    X( 16)
    #undef X

    default:
      ETL_ASSERT(false);
  }
}

float Rcc::get_cpu_clock_hz() const {
  return clock_speeds.cpu;
}

float Rcc::get_ahb_clock_hz() const {
  return clock_speeds.ahb;
}

float Rcc::get_apb1_clock_hz() const {
  return clock_speeds.apb1;
}

float Rcc::get_apb2_clock_hz() const {
  return clock_speeds.apb2;
}

float Rcc::get_pll48_clock_hz() const {
  return clock_speeds.pll48;
}


float Rcc::get_clock_hz(ApbPeripheral p) const {
  unsigned bus = get_bus_index(p);

  switch (bus) {
    case 0: return clock_speeds.apb1;
    case 1: return clock_speeds.apb2;

    default:
      ETL_ASSERT(false);
  }
}

void Rcc::configure_clocks(ClockConfig const &cfg) {
  cfg.compute_speeds(&clock_speeds);

  auto prescalers = cfgr_value_t()
                    .with_hpre(get_hpre(cfg.ahb_divisor))
                    .with_ppre1(get_ppre(cfg.apb1_divisor))
                    .with_ppre2(get_ppre(cfg.apb2_divisor));

  // Switch to the internal 16MHz oscillator while messing with the PLL.
  write_cr(read_cr().with_hsion(true));
  while (!read_cr().get_hsirdy());  // Wait for it to stabilize.

  write_cfgr(read_cfgr().with_sw(cfgr_value_t::sw_t::hsi));
  while (read_cfgr().get_sws() != cfgr_value_t::sws_t::hsi);  // Wait for it.

  // Turn off the PLL.
  write_cr(read_cr().with_pllon(false));
  while (read_cr().get_pllrdy());  // Wait for it to unlock.

  // Apply divisors before boosting frequency.
  write_cfgr(read_cfgr()
             .with_hpre(prescalers.get_hpre())
             .with_ppre1(prescalers.get_ppre1())
             .with_ppre2(prescalers.get_ppre2()));

  flash.write_acr(flash.read_acr().with_latency(cfg.flash_latency));

  // Switch on the crystal oscillator.
  write_cr(read_cr().with_hseon(true));
  while (!read_cr().get_hserdy());  // Wait for it.

  // Configure the PLL.

  auto pllp = pllcfgr_value_t::pllp_t((cfg.general_divisor >> 1) - 1);

  write_pllcfgr(read_pllcfgr()
                .with_pllm(cfg.crystal_divisor)
                .with_plln(cfg.vco_multiplier)
                .with_pllp(pllp)
                .with_pllq(cfg.pll48_divisor)
                .with_pllsrc(pllcfgr_value_t::pllsrc_t::hse));


  // Turn it on.
  write_cr(read_cr().with_pllon(true));
  while (!read_cr().get_pllrdy());

  // Select PLL as clock source.
  write_cfgr(read_cfgr().with_sw(cfgr_value_t::sw_t::pll));
  while (read_cfgr().get_sws() != cfgr_value_t::sws_t::pll);
}

void ClockConfig::compute_speeds(ClockSpeeds *out) const {
  float vco_in_hz = crystal_hz / static_cast<float>(crystal_divisor);
  float vco_out_hz = vco_in_hz * static_cast<float>(vco_multiplier);
  out->cpu = vco_out_hz / static_cast<float>(general_divisor);
  out->pll48 = vco_out_hz / static_cast<float>(pll48_divisor);

  out->ahb = out->cpu / static_cast<float>(ahb_divisor);
  out->apb1 = out->cpu / static_cast<float>(apb1_divisor);
  out->apb2 = out->cpu / static_cast<float>(apb2_divisor);
}

}  // namespace stm32f4xx
}  // namespace etl
