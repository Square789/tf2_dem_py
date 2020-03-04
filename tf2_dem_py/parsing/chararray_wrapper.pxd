from libc.stdio cimport FILE
from libc.stdint cimport uint32_t, uint8_t

cdef class CharArrayWrapper:
	cdef:
		uint8_t *mem_ptr
		uint32_t mem_len
		uint8_t bitbuf
		uint8_t bitbuf_len
		uint32_t pos

	@staticmethod
	cdef inline CharArrayWrapper create_new(FILE *file_ptr, uint32_t read_len)

	cdef uint32_t get_next_str_size(self)
	cdef str read_next_utf8_str(self)
	cdef str read_utf8_str(self, uint32_t req_len)
	cdef uint8_t *read_raw(self, uint32_t req_len)
