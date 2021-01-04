#include <stdint.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/parser_state/data_structs/game_events.h"
#include "tf2_dem_py/demo_parser/parser_state/data_structs/demo_header.h"

ParserState *ParserState_new() {
	ParserState *self = (ParserState *)malloc(sizeof(ParserState));
	if (self == NULL) {
		return NULL;
	}
	if (ParserState_init(self) != 0) {
		return NULL;
	};
	return self;
}

uint8_t ParserState_init(ParserState *self) {
	self->current_message = 255;
	self->flags = 0;
	self->finished = 0;
	self->failure = 0;
	self->RELAYED_CAW_ERR = 0;
	self->tick = 0;
	self->game_event_def_amount = 0;
	self->game_event_defs = NULL;
	self->game_events = NULL;
	self->game_events_amount = 0;
	self->game_events_capacity = 0;
	self->demo_header = DemoHeader_new();
	if (self->demo_header == NULL) { goto error0; }

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
			GameEventDefinition_destroy(self->game_event_defs + i);
		}
		free(self->game_event_defs);
		self->game_event_defs = NULL;
		self->game_event_def_amount = 0;
	}
}

void ParserState_free_game_events(ParserState *self) {
	if (self->game_events != NULL) {
		for (size_t i = 0; i < self->game_events_amount; i++) {
			GameEvent_destroy(self->game_events[i]);
		}
		free(self->game_events);
		self->game_events = NULL;
		self->game_events_amount = 0;
		self->game_events_capacity = 0;
	}
}

uint8_t ParserState_append_game_event(ParserState *self, GameEvent *ge) {
	MACRO_ARRAYLIST_APPEND(GameEvent *, self->game_events, self->game_events_capacity, self->game_events_amount)
	self->game_events[self->game_events_amount] = ge;
	// printf("Inserted game event %u @ %p\n", self->game_events_amount, self->game_events + self->game_events_amount);
	self->game_events_amount += 1;
	return 0;
}

uint8_t ParserState_read_DemoHeader(ParserState *self, FILE *stream) {
	CharArrayWrapper *header_caw = CharArrayWrapper_from_file(stream, 1072);

	if (self->demo_header == NULL) {
		self->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return 1;
	}

	if (header_caw->ERRORLEVEL != 0) {
		self->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
		self->failure |= ParserState_ERR_CAW;
		return 1;
	}

	self->demo_header->ident = CharArrayWrapper_get_chars(header_caw, 8);
	self->demo_header->net_prot = CharArrayWrapper_get_uint32(header_caw);
	self->demo_header->dem_prot = CharArrayWrapper_get_uint32(header_caw);
	self->demo_header->host_addr = CharArrayWrapper_get_chars_up_to_null(header_caw, 260);
	self->demo_header->client_id = CharArrayWrapper_get_chars_up_to_null(header_caw, 260);
	self->demo_header->map_name = CharArrayWrapper_get_chars_up_to_null(header_caw, 260);
	self->demo_header->game_dir = CharArrayWrapper_get_chars_up_to_null(header_caw, 260);
	self->demo_header->play_time = CharArrayWrapper_get_flt(header_caw);
	self->demo_header->tick_count = CharArrayWrapper_get_uint32(header_caw);
	self->demo_header->frame_count = CharArrayWrapper_get_uint32(header_caw);
	self->demo_header->sigon = CharArrayWrapper_get_uint32(header_caw);

	// Some of the demo header fields may be NULL if this if is entered, should be okay if init
	// was used and destroy will be called.
	if (header_caw->ERRORLEVEL |= 0) {
		self->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
		self->failure |= ParserState_ERR_CAW;
		return 1;
	}

	return 0;

}

const uint16_t ParserState_ERR_CAW = 1 << 0;
const uint16_t ParserState_ERR_UNKNOWN_PACKET_ID = 1 << 1;
const uint16_t ParserState_ERR_IO = 1 << 2;
const uint16_t ParserState_ERR_UNEXPECTED_EOF = 1 << 3;
const uint16_t ParserState_ERR_UNKNOWN_MESSAGE_ID = 1 << 4;
const uint16_t ParserState_ERR_MEMORY_ALLOCATION = 1 << 5;
const uint16_t ParserState_ERR_UNKNOWN_GAME_EVENT = 1 << 6;
const uint16_t ParserState_ERR_PYDICT = 1 << 7;
const uint16_t ParserState_ERR_PYLIST = 1 << 8;
const uint16_t ParserState_ERR_GAME_EVENT_INDEX_OUTBOUND = 1 << 9;
const uint16_t ParserState_ERR_UNKNOWN = 1 << 15;
