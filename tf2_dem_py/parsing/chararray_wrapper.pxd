from libc.stdio cimport FILE
from libc.stdint cimport uint32_t

cdef class CharArrayWrapper:
	# Needed to tell callers that this function does not want
	# python arguments
	@staticmethod
	cdef CharArrayWrapper create_new(FILE *file_ptr, uint32_t read_len)
