project('TranscodingVerification', 'c',
  version : '0.1',
  default_options : ['warning_level=3'])

# These arguments are only used to build the shared library
# not the executables that use the library.
lib_args = ['-DBUILDING_TRANSCODINGVERIFICATION']

# Hiding symbols that are not explicitly marked as exported
# requires a compiler flag on all compilers except VS.
cc = meson.get_compiler('c')
if cc.get_id() != 'msvc'
  lib_args += ['-fvisibility=hidden']
endif

dep_m = cc.find_library('m', required : false)

deps = [dep_m]

subdir('include')
subdir('src')
subdir('test')

# Make this library usable from the system's
# package manager.

pkg_mod = import('pkgconfig')
pkg_mod.generate(
  name : 'mea',
  filebase : 'mea',
  description : 'Transcoding Verification through Quality Metrics.',
  subdirs : 'mea',
  libraries : libmea,
  version : '0.1',
)
