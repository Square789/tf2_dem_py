from libc.stdint cimport uint8_t, uint16_t, uint32_t
from libc.stdio cimport FILE

cdef extern from "tf2_dem_py/char_array_wrapper/char_array_wrapper.h" nogil:
	cdef struct CharArrayWrapper:
		pass

	cdef CharArrayWrapper *CAW_create_new(FILE *fp, size_t initbytes)

	cdef void CAW_delete(CharArrayWrapper *caw)
	cdef void CAW_read_raw(CharArrayWrapper *caw, void *target_ptr, size_t req_bytes, uint8_t req_bits)
	cdef void CAW_skip(CharArrayWrapper *caw, size_t bytes_, uint8_t bits)
	cdef size_t CAW_dist_until_null(CharArrayWrapper *caw)
	cdef uint8_t CAW_remaining_bits(CharArrayWrapper *caw)
	cdef size_t CAW_remaining_bytes(CharArrayWrapper *caw)

	cdef uint8_t *CAW_get_nulltrm_str(CharArrayWrapper *caw)
	cdef uint8_t *CAW_get_chars(CharArrayWrapper *caw, size_t req_len)
	cdef float CAW_get_flt(CharArrayWrapper *caw)
	cdef uint8_t CAW_get_bit(CharArrayWrapper *caw)
	cdef uint8_t CAW_get_uint8(CharArrayWrapper *caw)
	cdef uint16_t CAW_get_uint16(CharArrayWrapper *caw)
	cdef uint32_t CAW_get_uint32(CharArrayWrapper *caw)

	cdef uint8_t CAW_get_errorlevel(CharArrayWrapper *caw)
