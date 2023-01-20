#include <stdint.h>
#include <stdlib.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

#include "tf2_dem_py/demo_parser/data_structs/game_events.h"
#include "tf2_dem_py/demo_parser/data_structs/demo_header.h"

ParserState *ParserState_new() {
	ParserState *self = (ParserState *)malloc(sizeof(ParserState));
	if (self == NULL) {
		goto fail0;
	}
	if (ParserState_init(self) != 0) {
		goto fail1;
	};
	return self;

fail1: free(self);
fail0:
	return NULL;
}

uint8_t ParserState_init(ParserState *self) {
	self->current_message = 255;
	self->flags = 0;
	self->finished = 0;
	self->failure = 0;
	self->relayed_caw_error = 0;
	self->tick = 0;
	self->game_event_defs = NULL;
	self->game_event_def_amount = 0;
	ArrayList_init(&self->game_events, sizeof(GameEvent));
	self->demo_header = DemoHeader_new();
	if (self->demo_header == NULL) {
		goto error0;
	}
	self->print_msg = NULL;
	self->server_info = NULL;
	ArrayList_init(&self->chat_messages, sizeof(ChatMessage *));

	return 0;

error0:
	return 1;
}

void ParserState_destroy(ParserState *self) {
	ParserState_free_game_event_defs(self);

	if (self->game_events.ptr != NULL) {
		for (size_t i = 0; i < self->game_events.len; i++) {
			GameEvent_free(((GameEvent *)self->game_events.ptr) + i);
		}
		ArrayList_destroy(&self->game_events);
	}

	DemoHeader_destroy(self->demo_header);

	if (self->server_info != NULL) {
		ServerInfo_destroy(self->server_info);
	}

	free(self->print_msg);

	if (self->chat_messages.ptr != NULL) {
		for (size_t i = 0; i < self->chat_messages.len; i++) {
			ChatMessage_destroy(((ChatMessage **)self->chat_messages.ptr)[i]);
		}
		ArrayList_destroy(&self->chat_messages);
	}

	free(self);
}

void ParserState_free_game_event_defs(ParserState *self) {
	if (self->game_event_defs != NULL) {
		for (size_t i = 0; i < self->game_event_def_amount; i++) {
			GameEventDefinition_free(self->game_event_defs + i);
		}
		free(self->game_event_defs);
		self->game_event_defs = NULL;
	}
	self->game_event_def_amount = 0;
}

uint8_t ParserState_append_game_event(ParserState *self, GameEvent ge) {
	if (ArrayList_check_size(&self->game_events) >= 2) {
		self->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return 1;
	}

	((GameEvent *)ArrayList_get_storage_pointer(&self->game_events))[0] = ge;
	return 0;
}

uint8_t ParserState_append_chat_message(ParserState *self, ChatMessage *cm) {
	if (ArrayList_check_size(&self->chat_messages) >= 2) {
		self->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return 1;
	}

	((ChatMessage **)ArrayList_get_storage_pointer(&self->chat_messages))[0] = cm;
	return 0;
}

void ParserState_read_demo_header(ParserState *self, FILE *stream) {
	CharArrayWrapper_err_t caw_err;
	switch (DemoHeader_read(self->demo_header, stream, &caw_err)) {
	case 1:
		self->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		break;
	case 2:
		self->failure |= ParserState_ERR_CAW;
		self->relayed_caw_error = caw_err;
		break;
	default:
		break;
	}
}

void ParserState_parse_packet(ParserState *self, FILE *stream) {

	uint8_t packet_type;
	if (fread(&packet_type, sizeof(packet_type), 1, stream) != 1) {
		self->failure |= ParserState_ERR_IO;
		return;
	}

	// printf("Next packet type: %u @%u\n", packet_type, ftell(stream));
	switch (packet_type) {
	case 1:
	case 2:
		Message_parse(stream, self); break;
	case 3:
		Synctick_parse(stream, self); break;
	case 4:
		Consolecmd_parse(stream, self); break;
	case 5:
		Usercmd_parse(stream, self); break;
	case 6:
		Datatables_parse(stream, self); break;
	case 7:
		self->finished = 1;
		break;
	case 8:
		Stringtables_parse(stream, self); break;
	default:
		self->failure |= ParserState_ERR_UNKNOWN_PACKET_ID;
	}
}


const ParserState_errflag_t ParserState_ERR_CAW = 1 << 0;
const ParserState_errflag_t ParserState_ERR_UNKNOWN_PACKET_ID = 1 << 1;
const ParserState_errflag_t ParserState_ERR_IO = 1 << 2;
const ParserState_errflag_t ParserState_ERR_UNEXPECTED_EOF = 1 << 3;
const ParserState_errflag_t ParserState_ERR_UNKNOWN_MESSAGE_ID = 1 << 4;
const ParserState_errflag_t ParserState_ERR_MEMORY_ALLOCATION = 1 << 5;
const ParserState_errflag_t ParserState_ERR_UNKNOWN_GAME_EVENT = 1 << 6;
const ParserState_errflag_t ParserState_ERR_PYDICT = 1 << 7;
const ParserState_errflag_t ParserState_ERR_PYLIST = 1 << 8;
const ParserState_errflag_t ParserState_ERR_GAME_EVENT_INDEX_OUTBOUND = 1 << 9;
const ParserState_errflag_t ParserState_ERR_UNKNOWN = 1 << 15;
