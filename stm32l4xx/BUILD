c_library('stm32l4xx',
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
    'c_link_flags': [ PKG + '/peripherals.ld' ],
  },
)

c_library('interrupt_table',
  sources = [
    'interrupt_table.cc',
  ],
  deps = [
    '//etl',
    '//etl/stm32l4xx',
  ],
  local = {
    'c_library_whole_archive': True,
  },
)
