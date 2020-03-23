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

	#printf("File ptr @%u, packet length %u\n", ftell(stream), pkt_len)

	cdef CharArrayWrapper *pkt_caw = CAW_from_file(stream, pkt_len)

	if CAW_get_errorlevel(pkt_caw) != 0:
		parser_state.FAILURE |= 0b1
		parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
		return

	while (CAW_remaining_bytes(pkt_caw) > 1) or (CAW_remaining_bits(pkt_caw) > 6):
		CAW_read_raw(pkt_caw, &msg_id, 0, 6)
		#printf(" -Next message: %u, tick %u\n", msg_id, parser_state.tick)
		if msg_id == 0:
			msg_parser = Empty
		elif msg_id == 2:
			msg_parser = File
		elif msg_id == 3:
			msg_parser = NetTick
		elif msg_id == 4:
			msg_parser = StringCommand
		elif msg_id == 5:
			msg_parser = SetConVar
		elif msg_id == 6:
			msg_parser = SigOnState
		elif msg_id == 7:
			msg_parser = Print
		elif msg_id == 8:
			msg_parser = ServerInfo
		elif msg_id == 10:
			msg_parser = ClassInfo
		elif msg_id == 12:
			msg_parser = StringTableCreate
		elif msg_id == 13:
			msg_parser = StringTableUpdate
		elif msg_id == 14:
			msg_parser = VoiceInit
		elif msg_id == 17:
			msg_parser = ParseSounds
		elif msg_id == 18:
			msg_parser = SetView
		elif msg_id == 19:
			msg_parser = FixAngle
		elif msg_id == 21:
			msg_parser = BspDecal
		elif msg_id == 23:
			msg_parser = UserMessage
		elif msg_id == 24:
			msg_parser = Entity
		elif msg_id == 25:
			msg_parser = GameEvent
		elif msg_id == 26:
			msg_parser = PacketEntities
		elif msg_id == 27:
			msg_parser = TempEntities
		elif msg_id == 28:
			msg_parser = PreFetch
		elif msg_id == 30:
			msg_parser = GameEventList
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
