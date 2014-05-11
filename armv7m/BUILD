c_library('armv7m',
  sources = [
    'nvic.cc',
    'scb.cc',
  ],
  deps = [
    '//etl',
  ],
  using = {
    'link_flags': [ PKG + '/peripherals.ld' ],
  },
)

c_library('exception_table',
  sources = [ 'exception_table.cc' ],
  deps = [
    '//etl/armv7m',
    '//etl',
  ],
  local = {
    'whole_archive': True,
  },
)

c_library('manual_crt0',
  sources = [ 'crt0.cc' ],
  deps = [
    '//etl/armv7m',
    '//etl',
  ],
  using = {
    # This library provides the equivalent of GCC's startfiles.  Attempting to
    # link in *both* will result in a symbol conflict.
    'link_flags': [ '-nostartfiles' ],
  },
)
