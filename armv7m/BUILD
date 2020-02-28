c_library('armv7m',
  sources = [
    'nvic.cc',
    'scb.cc',
  ],
  deps = [
    '//etl',
  ],
  using = {
    'c_link_flags': [ PKG + '/peripherals.ld' ],
  },
)

c_library('exception_table',
  sources = [ 'exception_table.cc' ],
  deps = [
    '//etl/armv7m',
    '//etl',
  ],
  local = {
    'c_library_whole_archive': True,
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
    'c_link_flags': [ '-nostartfiles' ],
  },
)

c_library('implicit_crt0',
  sources = [ 'implicit_crt0.cc' ],
  deps = [
    '//etl/armv7m:manual_crt0',
    '//etl/armv7m',
    '//etl',
  ],
  using = {
    'c_link_flags': [
      '-u main',
    ],
  },
)
