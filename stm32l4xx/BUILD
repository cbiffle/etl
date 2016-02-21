c_library('stm32l4xx',
  sources = [
    'gpio.cc',
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
