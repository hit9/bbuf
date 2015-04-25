{
  'targets': [{
    'target_name': 'buf',
    'sources': ['src/cc/bind.cc', 'src/cc/buf.cc'],
    'include_dirs': ["<!(node -e \"require('nan')\")"],
    'dependencies': ['src/c/buf.gyp:buf'],
    'defines': ['_GNU_SOURCE'],
    'cflags': ['-Wall', '-O3']
  }]
}
