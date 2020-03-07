from libc.stdio cimport FILE, fread, fseek, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

cdef void parse(FILE *stream):
	cdef uint32_t tick

	# That is all
	fread(&tick, sizeof(tick), 1, stream)
