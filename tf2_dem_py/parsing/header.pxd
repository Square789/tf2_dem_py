from libc.stdio cimport FILE

from tf2_dem_py.parsing.chararray_wrapper import CharArrayWrapper

cdef dict parse(FILE *stream)