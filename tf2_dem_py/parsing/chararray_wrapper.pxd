from libc.stdio cimport FILE
from libc.stdint cimport uint8_t, uint32_t, uint64_t

cdef class CharArrayWrapper:
	cdef:
		uint8_t *mem_ptr
		uint32_t mem_len
		uint8_t bitbuf
		uint8_t bitbuf_len
		uint32_t pos

	@staticmethod
	cdef CharArrayWrapper create_new(FILE *, size_t)

	cdef void _read_raw(self, void *, size_t, uint8_t)
	cdef uint8_t _ver_buf_health(self, size_t, uint8_t)
	cdef uint32_t dist_until_null(self)

	cdef str get_next_utf8_str(self)
	cdef str get_utf8_str(self, size_t)

	cdef uint64_t get_int(self, uint8_t)
	cdef uint32_t get_uint32(self)

	cdef float get_flt16(self)
	cdef double get_dbl32(self)
