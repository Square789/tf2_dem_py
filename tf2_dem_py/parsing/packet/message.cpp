#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/parsing/message/__init__.hpp"

inline uint8_t should_parse(uint8_t m_id, uint16_t flag) {
	if (m_id == 25) {
		return (flag & FLAGS.GAME_EVENTS);
	} else {
		return 1;
	}
}

void Message_parse(FILE *stream, ParserState *parser_state, PyObject *root_dict) {
	uint32_t tick;
	uint32_t pkt_len;
	uint8_t msg_id = 0;
	MessageParsers::IMsgParserBase *msg_parser;

	// Read tick
	fread(&tick, sizeof(tick), 1, stream);

	// Skip 84 bytes, containing some unimportant flag data
	fseek(stream, 84, SEEK_CUR);

	// Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		parser_state->FAILURE |= ParserState_ERR.IO;
		return;
	}
	if (feof(stream) != 0) {
		parser_state->FAILURE |= ParserState_ERR.UNEXPECTED_EOF;
		return;
	}

	// printf("Message packet, length %u\n", pkt_len);

	CharArrayWrapper *pkt_caw = CAW_from_file(stream, pkt_len);

	if (pkt_caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ParserState_ERR.CAW;
		parser_state->RELAYED_CAW_ERR = pkt_caw->ERRORLEVEL;
		return;
	}

	while ((pkt_caw->remaining_bytes() > 1) || (pkt_caw->remaining_bits() > 6)) {
		pkt_caw->read_raw(&msg_id, 0, 6);
		// printf(" -Next message: %u, tick %u\n", msg_id, parser_state.tick)
		switch (msg_id)
		{
		case 0:
			msg_parser = MessageParsers::cEmpty; break;
		case 2:
			msg_parser = MessageParsers::cFile; break;
		case 3:
			msg_parser = MessageParsers::cNetTick; break;
		case 4:
			msg_parser = MessageParsers::cStringCommand; break;
		case 5:
			msg_parser = MessageParsers::cSetConVar; break;
		case 6:
			msg_parser = MessageParsers::cSigOnState; break;
		case 7:
			msg_parser = MessageParsers::cPrint; break;
		case 8:
			msg_parser = MessageParsers::cServerInfo; break;
		case 10:
			msg_parser = MessageParsers::cClassInfo; break;
		case 12:
			msg_parser = MessageParsers::cStringTableCreate; break;
		case 13:
			msg_parser = MessageParsers::cStringTableUpdate; break;
		case 14:
			msg_parser = MessageParsers::cVoiceInit; break;
		case 15:
			msg_parser = MessageParsers::cVoiceData; break;
		case 17:
			msg_parser = MessageParsers::cParseSounds; break;
		case 18:
			msg_parser = MessageParsers::cSetView; break;
		case 19:
			msg_parser = MessageParsers::cFixAngle; break;
		case 21:
			msg_parser = MessageParsers::cBspDecal; break;
		case 23:
			msg_parser = MessageParsers::cUserMessage; break;
		case 24:
			msg_parser = MessageParsers::cEntity; break;
		case 25:
			msg_parser = MessageParsers::cGameEvent; break;
		case 26:
			msg_parser = MessageParsers::cPacketEntities; break;
		case 27:
			msg_parser = MessageParsers::cTempEntities; break;
		case 28:
			msg_parser = MessageParsers::cPreFetch; break;
		case 30:
			msg_parser = MessageParsers::cGameEventList; break;
		case 31:
			msg_parser = MessageParsers::cGetCvarValue; break;
		default:
			parser_state->FAILURE |= ParserState_ERR.UNKNOWN_MESSAGE_ID;
			return;
		}

		if (should_parse(msg_id, parser_state->flags)) {
			msg_parser->parse(pkt_caw, parser_state, root_dict);
		} else {
			msg_parser->skip(pkt_caw, parser_state);
		}

		if (pkt_caw->ERRORLEVEL != 0) {
			parser_state->FAILURE |= ParserState_ERR.CAW;
			parser_state->RELAYED_CAW_ERR = pkt_caw->ERRORLEVEL;
			return;
		}

		if (parser_state->FAILURE != 0) { // Set by message parser
			return;
		}
	}

	delete pkt_caw;
	parser_state->current_message_contains_senderless_chat = 0;
}
