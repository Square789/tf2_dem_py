#include <stdint.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

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
	self->RELAYED_CAW_ERR = 0;
	self->tick = 0;
	self->game_event_defs = NULL;
	self->game_event_def_amount = 0;
	self->game_events = NULL;
	self->game_events_capacity = 0;
	self->game_events_len = 0;
	self->demo_header = DemoHeader_new();
	if (self->demo_header == NULL) { goto error0; }
	self->print_msg = NULL;
	self->server_info = NULL;
	self->chat_messages = NULL;
	self->chat_messages_capacity = 0;
	self->chat_messages_len = 0;

	return 0;

error0:
	return 1;
}

void ParserState_destroy(ParserState *self) {
	ParserState_free_game_event_defs(self);
	ParserState_free_game_events(self);
	DemoHeader_destroy(self->demo_header);
	free(self);
}

void ParserState_free_game_event_defs(ParserState *self) {
	if (self->game_event_defs != NULL) {
		for (size_t i = 0; i < self->game_event_def_amount; i++) {
			GameEventDefinition_free(self->game_event_defs + i);
		}
		free(self->game_event_defs);
		self->game_event_defs = NULL;
		self->game_event_def_amount = 0;
	}
}

void ParserState_free_game_events(ParserState *self) {
	if (self->game_events != NULL) {
		for (size_t i = 0; i < self->game_events_len; i++) {
			GameEvent_destroy(self->game_events[i]);
		}
		free(self->game_events);
		self->game_events = NULL;
		self->game_events_len = 0;
		self->game_events_capacity = 0;
	}
}

uint8_t ParserState_append_game_event(ParserState *self, GameEvent *ge) {
	if (_generic_arraylist_size_check(sizeof(GameEvent *), &self->game_events, &self->game_events_capacity,
			&self->game_events_len) != 0) {
		return 1;
	}
	self->game_events[self->game_events_len] = ge;
	// printf("Inserted game event %u @ %p\n", self->game_events_len, self->game_events + self->game_events_len);
	self->game_events_len += 1;
	return 0;
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
