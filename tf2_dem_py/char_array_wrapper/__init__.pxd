# distutils: language = c++

from libc.stdint cimport uint8_t, uint16_t, uint32_t
from libc.stdio cimport FILE

cdef extern from "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp" nogil:
	# (Lacks caw_from_caw methods, but not used in cython so who cares)
	cdef cppclass CharArrayWrapper:
		char *mem_ptr
		size_t mem_len
		uint8_t bitbuf
		uint8_t bitbuf_len
		size_t bytepos
		uint8_t ERRORLEVEL
		uint8_t free_on_dealloc

		CharArrayWrapper(char *mem_ptr, size_t mem_len) except +
		#~CharArrayWrapper() except + # Apparently not required

		void read_raw(void *target_ptr, size_t req_bytes, uint8_t req_bits)
		void skip(size_t bytes_, uint8_t bits)
		size_t get_pos_byte()
		uint8_t get_pos_bit()
		void set_pos(size_t byte, uint8_t bit)
		size_t dist_until_null()
		size_t remaining_bytes()
		uint8_t remaining_bits()
		char *get_nulltrm_str()

		char *get_chars(size_t req_len)
		float get_flt()
		uint8_t get_bit()
		uint8_t get_uint8()
		uint16_t get_uint16()
		uint32_t get_uint32()

		uint8_t get_errorlevel()

	CharArrayWrapper *CAW_from_file(FILE *fp, size_t initbytes)

