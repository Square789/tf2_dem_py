from libc.stdint cimport uint8_t, uint32_t
from libc.stdio cimport FILE

cdef extern from "char_array_wrapper.h":
	cdef struct CharArrayWrapper:
		pass

	cdef CharArrayWrapper *CAW_create_new(FILE *fp, size_t initbytes)

	cdef void CAW_delete(CharArrayWrapper *caw)
	cdef void CAW_read_raw(CharArrayWrapper *caw, void *target_ptr, size_t req_bytes, uint8_t req_bits)

	cdef uint8_t *CAW_get_chars(CharArrayWrapper *caw, size_t req_len)
	cdef float CAW_get_flt(CharArrayWrapper *caw)
	cdef uint32_t CAW_get_uint32(CharArrayWrapper *caw)

	cdef uint8_t CAW_get_errorlevel(CharArrayWrapper *caw)
