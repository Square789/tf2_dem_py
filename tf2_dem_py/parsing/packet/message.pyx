from libc.stdio cimport FILE, fread, fseek, ftell, ferror, feof, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.parsing.chararray_wrapper import CharArrayWrapper
#from tf2_dem_py.parsing.message.parse_any cimport parse_any
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef void parse(FILE *stream, ParserState *p_state, cJSON *root_json):
	cdef uint32_t tick
	cdef uint32_t pkt_len

	# Read tick
	fread(&tick, sizeof(tick), 1, stream)

	# Skip 84 bytes, always null
	fseek(stream, 84, SEEK_CUR)

	# Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream)

	if ferror(stream) != 0:
		p_state.FAILURE |= 0b100
		return
	if feof(stream) != 0:
		p_state.FAILURE |= 0b1000
		return

	fseek(stream, pkt_len, SEEK_CUR)
	# cdef CharArrayWrapper pkt_stream = CharArrayWrapper.create_mew(
	# 	stream, pkt_len)

	# if pkt_stream.ERRORLEVEL != 0:
	# 	self.p_state.FAILURE |= 0b1
	# 	self.p_state.RELAYED_CAW_ERR = pkt_stream.ERRORLEVEL
	# 	return

	# while : # wörk
	# 	parse_any(pkt_stream)
