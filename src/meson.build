mea_sources = [ 'mea_session.c', 'mea.c', 'msssim.c', 'psnr.c' ]

libmea = static_library('mea', mea_sources, include_directories
                       : inc, dependencies
                       : deps, install
                       : true, c_args
                       : lib_args, )
