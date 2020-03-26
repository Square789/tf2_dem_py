from libc.stdint cimport uint8_t, uint16_t, uint32_t
from libc.stdio cimport FILE

cdef extern from "tf2_dem_py/char_array_wrapper/char_array_wrapper.h" nogil:
	cdef struct CharArrayWrapper:
		pass

	CharArrayWrapper *CAW_from_file(FILE *fp, size_t initbytes)

	void CAW_delete(CharArrayWrapper *caw)
	void CAW_read_raw(CharArrayWrapper *caw, void *target_ptr, size_t req_bytes, uint8_t req_bits)
	void CAW_skip(CharArrayWrapper *caw, size_t bytes_, uint8_t bits)
	size_t CAW_get_pos_byte(CharArrayWrapper *caw)
	uint8_t CAW_get_pos_bit(CharArrayWrapper *caw)
	void CAW_set_pos(CharArrayWrapper *caw, size_t byte, uint8_t bit)
	size_t CAW_dist_until_null(CharArrayWrapper *caw)
	uint8_t CAW_remaining_bits(CharArrayWrapper *caw)
	size_t CAW_remaining_bytes(CharArrayWrapper *caw)

	uint8_t *CAW_get_nulltrm_str(CharArrayWrapper *caw)
	uint8_t *CAW_get_chars(CharArrayWrapper *caw, size_t req_len)
	float CAW_get_flt(CharArrayWrapper *caw)
	uint8_t CAW_get_bit(CharArrayWrapper *caw)
	uint8_t CAW_get_uint8(CharArrayWrapper *caw)
	uint16_t CAW_get_uint16(CharArrayWrapper *caw)
	uint32_t CAW_get_uint32(CharArrayWrapper *caw)

	uint8_t CAW_get_errorlevel(CharArrayWrapper *caw)
