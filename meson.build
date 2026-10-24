project('dysk-c', 'c',
  default_options: [
    'c_std=gnu17',
    'warning_level=3',
  ],
)

executable('dsc',
  sources: [
    'src/main.c',
    'src/dysk_c.c',
    'src/strutil.c',
  ],
)

tests = executable('tests',
  sources: [
    'tests.c',
    'src/dysk_c.c',
    'src/strutil.c',
  ],
  dependencies: dependency('check'),
)

test('tests', tests)
