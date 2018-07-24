mea_sources = [ 'mea_frame.c', 'mea_session.c', 'mea.c' ]

shlib = static_library('mea', mea_sources, include_directories
                       : inc, dependencies
                       : deps, install
                       : true, c_args
                       : lib_args, )
