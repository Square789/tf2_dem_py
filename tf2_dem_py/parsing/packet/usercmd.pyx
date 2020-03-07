from libc.stdio cimport FILE, fread, fseek, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

cdef void parse(FILE *stream):
	cdef uint32_t tick
	cdef uint32_t seq_out
	cdef uint32_t pkt_len

	# Read tick of packet
	fread(&tick, sizeof(tick), 1, stream)

	# Read seq_out (whatever that is)
	fread(&seq_out, sizeof(seq_out), 1, stream)

	# Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream)

	# Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR)
