from libc.stdio cimport FILE, fread, fseek, ferror, feof, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState

cdef void parse(FILE *stream, ParserState *p_state):
	cdef uint32_t tick
	cdef uint32_t pkt_len

	# Read tick of packet data
	fread(&tick, sizeof(tick), 1, stream)

	# Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream)

	if ferror(stream) != 0:
		p_state.FAILURE |= 0b100
		return
	if feof(stream) != 0:
		p_state.FAILURE |= 0b1000
		return

	# Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR)
