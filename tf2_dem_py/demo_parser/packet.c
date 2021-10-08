#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/demo_parser/parser_state.h"
#include "tf2_dem_py/demo_parser/message/__init__.h"


#define ALIAS_SKIP(name) void name(FILE *stream, ParserState *parser_state) { _packet_skip(stream, parser_state); }

void _packet_skip(FILE *stream, ParserState *parser_state) {
	uint32_t tick;
	uint32_t pkt_len;

	// Read tick of packet data
	fread(&tick, sizeof(tick), 1, stream);

	// Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		parser_state->failure |= ParserState_ERR_IO;
		return;
	}
	if (feof(stream) != 0) {
		parser_state->failure |= ParserState_ERR_UNEXPECTED_EOF;
		return;
	}

	// Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR);
}

ALIAS_SKIP(Consolecmd_parse)
ALIAS_SKIP(Datatables_parse)
ALIAS_SKIP(Stringtables_parse)

void Usercmd_parse(FILE *stream, ParserState *parser_state) {
	uint32_t tick;
	uint32_t pkt_len;

	fread(&tick, sizeof(tick), 1, stream);
	fseek(stream, 4, SEEK_CUR); // Usercmd requires this, skips "sequence_out"

	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		parser_state->failure |= ParserState_ERR_IO;
		return;
	}
	if (feof(stream) != 0) {
		parser_state->failure |= ParserState_ERR_UNEXPECTED_EOF;
		return;
	}

	fseek(stream, pkt_len, SEEK_CUR);
}

bool Message_should_parse(uint8_t m_id, flag_t flags) {
	switch (m_id) {
	case 25:
		return (flags & FLAGS_GAME_EVENTS) ? true : false;
	default:
		return true;
	}
}

void Message_parse(FILE *stream, ParserState *parser_state) {
	uint32_t tick;
	uint32_t pkt_len;
	uint8_t msg_id = 0;
	const MsgParser *msg_parser;

	// Read tick
	fread(&tick, sizeof(tick), 1, stream);

	// Skip 84 bytes, containing some unimportant flag data
	fseek(stream, 84, SEEK_CUR);

	// Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		parser_state->failure |= ParserState_ERR_IO;
		goto error0;
	}
	if (feof(stream) != 0) {
		parser_state->failure |= ParserState_ERR_UNEXPECTED_EOF;
		goto error0;
	}

	// printf("Message packet, length %u, fptr @%d\n", pkt_len, ftell(stream));

	CharArrayWrapper *pkt_caw = CharArrayWrapper_from_file(stream, pkt_len);

	if (pkt_caw->ERRORLEVEL != 0) {
		parser_state->failure |= ParserState_ERR_CAW;
		parser_state->RELAYED_CAW_ERR = pkt_caw->ERRORLEVEL;
		goto error1;
	}

	while ((CharArrayWrapper_remaining_bytes(pkt_caw) > 1) || (CharArrayWrapper_remaining_bits(pkt_caw) > 6)) {
		CharArrayWrapper_read_raw(pkt_caw, &msg_id, 0, 6);
		parser_state->current_message = msg_id;
		// printf(" -Next message: %u, tick %u, %d bytes in\n", msg_id, parser_state->tick, CharArrayWrapper_get_pos_byte(pkt_caw));
		switch (msg_id) {
		case 0:
			msg_parser = &MsgParser_Empty; break;
		case 2:
			msg_parser = &MsgParser_File; break;
		case 3:
			msg_parser = &MsgParser_NetTick; break;
		case 4:
			msg_parser = &MsgParser_StringCommand; break;
		case 5:
			msg_parser = &MsgParser_SetConVar; break;
		case 6:
			msg_parser = &MsgParser_SigOnState; break;
		case 7:
			msg_parser = &MsgParser_Print; break;
		case 8:
			msg_parser = &MsgParser_ServerInfo; break;
		case 10:
			msg_parser = &MsgParser_ClassInfo; break;
		case 12:
			msg_parser = &MsgParser_StringTableCreate; break;
		case 13:
			msg_parser = &MsgParser_StringTableUpdate; break;
		case 14:
			msg_parser = &MsgParser_VoiceInit; break;
		case 15:
			msg_parser = &MsgParser_VoiceData; break;
		case 17:
			msg_parser = &MsgParser_ParseSounds; break;
		case 18:
			msg_parser = &MsgParser_SetView; break;
		case 19:
			msg_parser = &MsgParser_FixAngle; break;
		case 21:
			msg_parser = &MsgParser_BspDecal; break;
		case 23:
			msg_parser = &MsgParser_UserMessage; break;
		case 24:
			msg_parser = &MsgParser_Entity; break;
		case 25:
			msg_parser = &MsgParser_GameEvent; break;
		case 26:
			msg_parser = &MsgParser_PacketEntities; break;
		case 27:
			msg_parser = &MsgParser_TempEntities; break;
		case 28:
			msg_parser = &MsgParser_PreFetch; break;
		case 30:
			msg_parser = &MsgParser_GameEventList; break;
		case 31:
			msg_parser = &MsgParser_GetCvarValue; break;
		default:
			parser_state->failure |= ParserState_ERR_UNKNOWN_MESSAGE_ID;
			goto error1;
		}

		if (Message_should_parse(msg_id, parser_state->flags)) {
			msg_parser->parse(pkt_caw, parser_state);
		} else {
			msg_parser->skip(pkt_caw, parser_state);
		}

		if (pkt_caw->ERRORLEVEL != 0) {
			parser_state->failure |= ParserState_ERR_CAW;
			parser_state->RELAYED_CAW_ERR = pkt_caw->ERRORLEVEL;
			goto error1;
		}

		if (parser_state->failure != 0) { // Set by message parser
			goto error1;
		}
	}

error1: CharArrayWrapper_destroy(pkt_caw);
error0:
	return;
}

void Synctick_parse(FILE *stream, ParserState *p_state) {
	uint32_t tick;

	// That is all
	fread(&tick, sizeof(tick), 1, stream);
}
