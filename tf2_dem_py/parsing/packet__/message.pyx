from libc.stdio cimport FILE, fread, fseek, ftell, ferror, feof, SEEK_CUR, printf
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.char_array_wrapper cimport *
from tf2_dem_py.cJSON cimport cJSON
from tf2_dem_py.flags cimport FLAGS
from tf2_dem_py.parsing.parser_state cimport ParserState, ERR
from tf2_dem_py.parsing.message cimport IMsgParserBase, cEmpty, cFile, cNetTick, \
	cStringCommand, cSetConVar, cSigOnState, cPrint, cServerInfo, cClassInfo, \
	cStringTableCreate, cStringTableUpdate, cVoiceInit, cVoiceData, cParseSounds, \
	cSetView, cFixAngle, cBspDecal, cUserMessage, cEntity, cGameEvent, cPacketEntities, \
	cTempEntities, cPreFetch, cGameEventList, cGetCvarValue

cdef void parse(FILE *stream, ParserState *parser_state, cJSON *root_json):
	cdef uint32_t tick
	cdef uint32_t pkt_len
	cdef uint8_t msg_id = 0
	cdef IMsgParserBase *msg_parser

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

	if pkt_caw.ERRORLEVEL != 0:
		parser_state.FAILURE |= ERR.CAW
		parser_state.RELAYED_CAW_ERR = pkt_caw.ERRORLEVEL
		return

	while (pkt_caw.remaining_bytes() > 1) or (pkt_caw.remaining_bits() > 6):
		pkt_caw.read_raw(&msg_id, 0, 6)
		# printf(" -Next message: %u, tick %u\n", msg_id, parser_state.tick)
		if msg_id == 0: # YandereDev style
			msg_parser = cEmpty
		elif msg_id == 2:
			msg_parser = cFile
		elif msg_id == 3:
			msg_parser = cNetTick
		elif msg_id == 4:
			msg_parser = cStringCommand
		elif msg_id == 5:
			msg_parser = cSetConVar
		elif msg_id == 6:
			msg_parser = cSigOnState
		elif msg_id == 7:
			msg_parser = cPrint
		elif msg_id == 8:
			msg_parser = cServerInfo
		elif msg_id == 10:
			msg_parser = cClassInfo
		elif msg_id == 12:
			msg_parser = cStringTableCreate
		elif msg_id == 13:
			msg_parser = cStringTableUpdate
		elif msg_id == 14:
			msg_parser = cVoiceInit
		elif msg_id == 15:
			msg_parser = cVoiceData
		elif msg_id == 17:
			msg_parser = cParseSounds
		elif msg_id == 18:
			msg_parser = cSetView
		elif msg_id == 19:
			msg_parser = cFixAngle
		elif msg_id == 21:
			msg_parser = cBspDecal
		elif msg_id == 23:
			msg_parser = cUserMessage
		elif msg_id == 24:
			msg_parser = cEntity
		elif msg_id == 25:
			msg_parser = cGameEvent
		elif msg_id == 26:
			msg_parser = cPacketEntities
		elif msg_id == 27:
			msg_parser = cTempEntities
		elif msg_id == 28:
			msg_parser = cPreFetch
		elif msg_id == 30:
			msg_parser = cGameEventList
		elif msg_id == 31:
			msg_parser = cGetCvarValue
		else:
			parser_state.FAILURE |= ERR.UNKNOWN_MESSAGE_ID
			return

		if should_parse(msg_id, parser_state.flags):
			msg_parser.parse(pkt_caw, parser_state, root_json)
		else:
			msg_parser.skip(pkt_caw, parser_state)

		if pkt_caw.ERRORLEVEL != 0:
			parser_state.FAILURE |= ERR.CAW
			parser_state.RELAYED_CAW_ERR = pkt_caw.ERRORLEVEL
			return

		if parser_state.FAILURE != 0: # Set by message parser
			return

	del pkt_caw # C++ deletion, hopefully it works the way it should.
	parser_state.current_message_contains_senderless_chat = 0

cdef inline uint8_t should_parse(uint8_t m_id, uint16_t flag) nogil:
	if m_id == 25:
		if flag & FLAGS.GAME_EVENTS:
			return 1
		else:
			return 0
	else:
		return 1
