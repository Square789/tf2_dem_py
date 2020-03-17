from libc.stdio cimport FILE, fread, fseek, ftell, ferror, feof, SEEK_CUR, printf
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.char_array_wrapper cimport *
from tf2_dem_py.parsing.message cimport *
from tf2_dem_py.cJSON cimport cJSON

cdef void parse(FILE *stream, ParserState *parser_state, cJSON *root_json):
	cdef uint32_t tick
	cdef uint32_t pkt_len
	cdef uint8_t msg_id = 0
	cdef MsgParserBase *msg_parser

	# Read tick
	fread(&tick, sizeof(tick), 1, stream)

	# Skip 84 bytes, always null
	fseek(stream, 84, SEEK_CUR)

	# Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream)

	if ferror(stream) != 0:
		parser_state.FAILURE |= 0b100
		return
	if feof(stream) != 0:
		parser_state.FAILURE |= 0b1000
		return

	printf("File ptr @%u\n", ftell(stream))

	cdef CharArrayWrapper *pkt_caw = CAW_create_new(stream, pkt_len)

	if CAW_get_errorlevel(pkt_caw) != 0:
		parser_state.FAILURE |= 0b1
		parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
		return

	while (CAW_remaining_bytes(pkt_caw) > 1) or (CAW_remaining_bits(pkt_caw) > 6):
		CAW_read_raw(pkt_caw, &msg_id, 0, 6)
		printf("njoy %u\n", msg_id)
		if msg_id == 0:
			msg_parser = Empty
		elif msg_id == 3:
			msg_parser = NetTick
		elif msg_id == 7:
			msg_parser = Print
		elif msg_id == 8:
			msg_parser = ServerInfo
		elif msg_id == 12:
			msg_parser = StringTableCreate
		else:
			parser_state.FAILURE |= 0b100000
			return

		msg_parser.parse(pkt_caw, parser_state, root_json)

		if CAW_get_errorlevel(pkt_caw) != 0:
			parser_state.FAILURE |= 0b1
			parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
			return

		if parser_state.FAILURE != 0: # Set by message parser
			return
