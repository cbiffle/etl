c_library('etl')

c_library('assert_loop',
  sources = [ 'assert_loop.cc' ],
  deps = [ ':etl' ],
  local = {
    'c_library_whole_archive': True,
  },
)
