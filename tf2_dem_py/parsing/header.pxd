from libc.stdio cimport FILE

from chararray_wrapper import CharArrayWrapper

cdef dict parse(FILE *stream)