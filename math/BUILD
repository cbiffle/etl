c_library('math',
  sources = [
    'trig.cc',
  ],
  local = {
    'cxx_flags': [ '-O2' ],
  },
  deps = [
    '//etl',
  ],
).extend_when(lambda e: e.get('etl_config_use_toolchain_trig'),
  local = {
    'cxx_flags': [ '-DETL_CONFIG_USE_TOOLCHAIN_TRIG' ],
  },
  using = {
    'cxx_flags': [ '-DETL_CONFIG_USE_TOOLCHAIN_TRIG' ],
  },
)
