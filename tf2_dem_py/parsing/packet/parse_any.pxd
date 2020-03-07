from libc.stdio cimport FILE

cdef dict parse_any(FILE *stream, char *finish_flag)
