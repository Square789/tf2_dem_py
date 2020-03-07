from libc.stdio cimport FILE, fread, fseek, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

cdef void parse(FILE *stream):
	cdef uint8_t tick
	cdef uint32_t pkt_len = 0

	# Skip 88 bytes, always null
	fseek(stream, 84, SEEK_CUR)

	# Read length of packet data
	fread(&pkt_len, sizeof(uint32_t), 1, stream)

	# Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR)
