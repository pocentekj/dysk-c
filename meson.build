project('dysk-c', 'c',
  default_options: [
    'c_std=gnu17',
    'warning_level=3',
  ],
)

cc = meson.get_compiler('c')

dep_unistring = cc.find_library('unistring')
dep_check = dependency('check')

executable('dsc',
  sources: [
    'src/main.c',
    'src/dysk_c.c',
    'src/strutil.c',
  ],
  dependencies: dep_unistring,
)

tests = executable('tests',
  sources: [
    'tests.c',
    'src/dysk_c.c',
    'src/strutil.c',
  ],
  dependencies: [dep_unistring, dep_check],
)

test('tests', tests)
