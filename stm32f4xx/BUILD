c_library('stm32f4xx',
  sources = [
    'gpio.cc',
    'interrupts.cc',
    'rcc.cc',
  ],
  deps = [
    '//etl/armv7m',
    '//etl',
  ],
  using = {
    'link_flags': [ PKG + '/peripherals.ld' ],
  },
)

c_library('interrupt_table',
  sources = [
    'interrupt_table.cc',
  ],
  deps = [
    '//etl',
    '//etl/stm32f4xx',
  ],
  local = {
    'whole_archive': True,
  },
)
