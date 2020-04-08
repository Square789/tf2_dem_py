from libc.stdio cimport FILE, fread, fseek, ftell, ferror, feof, SEEK_CUR, printf
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.char_array_wrapper cimport *
from tf2_dem_py.cJSON cimport cJSON
from tf2_dem_py.flags cimport FLAGS
from tf2_dem_py.parsing.parser_state cimport ParserState, ERR
from tf2_dem_py.parsing.message cimport *

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
		parser_state.FAILURE |= ERR.IO
		return
	if feof(stream) != 0:
		parser_state.FAILURE |= ERR.UNEXCPECTED_EOF
		return

	#printf("Message packet, length %u\n", pkt_len)

	cdef CharArrayWrapper *pkt_caw = CAW_from_file(stream, pkt_len)

	if CAW_get_errorlevel(pkt_caw) != 0:
		parser_state.FAILURE |= ERR.CAW
		parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
		return

	while (CAW_remaining_bytes(pkt_caw) > 1) or (CAW_remaining_bits(pkt_caw) > 6):
		CAW_read_raw(pkt_caw, &msg_id, 0, 6)
		# printf(" -Next message: %u, tick %u\n", msg_id, parser_state.tick)
		if msg_id == 0: # YandereDev style
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
		elif msg_id == 15:
			msg_parser = VoiceData
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
		elif msg_id == 31:
			msg_parser = GetCvarValue
		else:
			parser_state.FAILURE |= ERR.UNKNOWN_MESSAGE_ID
			return

		if should_parse(msg_id, parser_state.flags):
			msg_parser.parse(pkt_caw, parser_state, root_json)
		else:
			msg_parser.skip(pkt_caw, parser_state)

		if CAW_get_errorlevel(pkt_caw) != 0:
			parser_state.FAILURE |= ERR.CAW
			parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(pkt_caw)
			return

		if parser_state.FAILURE != 0: # Set by message parser
			return

	CAW_delete(pkt_caw)
	parser_state.current_message_contains_senderless_chat = 0

cdef inline uint8_t should_parse(uint8_t m_id, uint16_t flag) nogil:
	if m_id == 25:
		if flag & FLAGS.GAME_EVENTS:
			return 1
		else:
			return 0
	else:
		return 1
