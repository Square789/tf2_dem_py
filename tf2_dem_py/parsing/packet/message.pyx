from libc.stdio cimport FILE, fread, fseek, ftell, ferror, feof, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport *
from tf2_dem_py.parsing.message.parse_any cimport parse_any as message_parse_any
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef void parse(FILE *stream, ParserState *p_state, cJSON *root_json):
	cdef uint32_t tick
	cdef uint32_t pkt_len
	cdef uint8_t msg_id = 0

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

	# fseek(stream, pkt_len, SEEK_CUR)
	print("File ptr @", ftell(stream))

	cdef CharArrayWrapper *pkt_caw = CAW_create_new(stream, pkt_len)

	if CAW_get_errorlevel(pkt_caw) != 0:
		p_state.FAILURE |= 0b1
		p_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
		return

	while (CAW_remaining_bytes(pkt_caw) > 1) or (CAW_remaining_bits(pkt_caw) > 6):
		CAW_read_raw(pkt_caw, &msg_id, 0, 6)
		print("    Next message:", msg_id)
		if msg_id == 1:
			print("     -Message bad, exiting")
			return
		else:
			print("     -No parser for message with id", msg_id)
			return

