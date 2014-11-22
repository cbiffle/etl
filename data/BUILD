c_library('data',
  deps = [
    '//etl',
  ],
)

for name, l2size in [ ['crc32_fast', '8'],
                      ['crc32_small', '4'] ]:
  c_library(name,
    sources = [
      'crc32.cc',
    ],
    local = {
      'cxx_flags': [ '-DETL_DATA_CRC32_TABLE_L2SIZE=' + l2size ],
    },
    deps = [
      '//etl',
      '//etl/data',
    ],
  )
