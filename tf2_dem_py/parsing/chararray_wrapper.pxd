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
	cdef CharArrayWrapper create_new(FILE *file_ptr, size_t read_len)

	cdef void _check_for_space(self)
	cdef uint8_t *get_raw(self, size_t req_len)
	cdef uint32_t get_next_str_size(self)
	cdef str get_next_utf8_str(self)
	cdef str get_utf8_str(self, size_t req_len)
	cdef uint32_t get_uint32(self)
