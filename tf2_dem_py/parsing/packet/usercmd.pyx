from libc.stdio cimport FILE, fread, fseek, ferror, feof, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef void parse(FILE *stream, ParserState *p_state, cJSON *root_json):
	cdef uint32_t tick
	cdef uint32_t seq_out
	cdef uint32_t pkt_len

	# Read tick of packet
	fread(&tick, sizeof(tick), 1, stream)

	# Read seq_out (whatever that is)
	fread(&seq_out, sizeof(seq_out), 1, stream)

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
