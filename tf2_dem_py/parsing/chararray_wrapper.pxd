from libc.stdio cimport FILE
from libc.stdint cimport uint8_t, uint32_t, uint64_t

cdef class CharArrayWrapper:
	cdef:
		uint8_t *mem_ptr
		uint32_t mem_len
		uint8_t bitbuf
		uint8_t bitbuf_len
		uint32_t pos
		uint8_t ERRORLEVEL

	@staticmethod
	cdef CharArrayWrapper create_new(FILE *, size_t)

	cdef void _read_raw(self, void *file_ptr, size_t, uint8_t read_len)
	cdef uint8_t _ver_buf_health(self, size_t req_bytes, uint8_t req_bits)
	cdef uint32_t dist_until_null(self)

	cdef uint8_t *get_chars(self, size_t req_len)
	cdef str get_next_utf8_str(self)
	cdef str get_utf8_str(self, size_t req_len)

	cdef uint64_t get_int(self, uint8_t req_bits)
	cdef uint32_t get_uint32(self)

	cdef float get_flt32(self)
	cdef double get_dbl64(self)
