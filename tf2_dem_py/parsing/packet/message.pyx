from libc.stdio cimport FILE, fread, fseek, ftell, SEEK_CUR, printf, feof, ferror
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState

cdef void parse(FILE *stream, ParserState *p_state):
	cdef uint32_t tick
	cdef uint32_t pkt_len

	# Read tick
	fread(&tick, sizeof(tick), 1, stream)

	# Skip 84 bytes, always null
	fseek(stream, 84, SEEK_CUR)

	# Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream)

	# Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR)
