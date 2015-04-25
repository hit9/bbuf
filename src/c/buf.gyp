{
  'targets': [{
    'target_name': 'buf',
      'type': 'static_library',
      'direct_dependent_settings': {
        'include_dirs': [ '.'  ],
      },
      'sources': ['./buf.c'],
      'conditions': [
        ['OS=="mac"', {'xcode_settings': {'GCC_C_LANGUAGE_STANDARD': 'c99'}}],
        ['OS=="solaris"', {'cflags+': [ '-std=c99']}]
      ]
  }]
}
