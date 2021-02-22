#ifndef PARSER_STATE__H
#define PARSER_STATE__H

#include <stdint.h>
#include "tf2_dem_py/demo_parser/data_structs/game_events.h"
#include "tf2_dem_py/demo_parser/data_structs/demo_header.h"

// A ParserState will hold all of the data extracted from a demo and a state
// during parsing as well.
typedef struct {
	// ID of the currently parsed Message
	uint8_t current_message;
	// Flags set to the parser, see tf2_dem_py/flags
	uint32_t flags;
	// Whether the parser is done.
	uint8_t finished;
	// Error bits
	uint8_t failure;
	// Error bits of an underlying CharArrayWrapper, if error was caused by it.
	uint8_t RELAYED_CAW_ERR;
	// Current tick in the demo
	uint32_t tick;
	// GameEvent definitions. These may be NULL earlier in the parsing process, will be
	// set to a static length array as soon as a GameEventList message is encountered.
	GameEventDefinition *game_event_defs;
	size_t game_event_def_amount;
	// Game Events. This is a dynamically reallocated array of game events occurring in the demo.
	GameEvent **game_events;
	size_t game_events_capacity;
	size_t game_events_amount;
	// Demo header
	DemoHeader *demo_header;
	// The string of the first found Print message (there should only be one) or NULL
	uint8_t *print_msg;
	// The string of the first found ServerInfo message (there should only be one) or NULL
	uint8_t *server_info;
} ParserState;

ParserState *ParserState_new();
uint8_t ParserState_init(ParserState *self);
void ParserState_destroy(ParserState *self);

// If it is not NULL, destroys all game even definitions, frees the
// game_event_definition array and sets its size to 0.
void ParserState_free_game_event_defs(ParserState *self);

// If it is not NULL, frees all game events, frees their array and sets the associated
// fields back to NULL / 0.
void ParserState_free_game_events(ParserState *self);

uint8_t ParserState_append_game_event(ParserState *self, GameEvent *ge);

extern const uint16_t ParserState_ERR_CAW;
extern const uint16_t ParserState_ERR_UNKNOWN_PACKET_ID;
extern const uint16_t ParserState_ERR_IO;
extern const uint16_t ParserState_ERR_UNEXPECTED_EOF;
extern const uint16_t ParserState_ERR_UNKNOWN_MESSAGE_ID;
extern const uint16_t ParserState_ERR_MEMORY_ALLOCATION;
extern const uint16_t ParserState_ERR_UNKNOWN_GAME_EVENT;
extern const uint16_t ParserState_ERR_PYDICT;
extern const uint16_t ParserState_ERR_PYLIST;
extern const uint16_t ParserState_ERR_GAME_EVENT_INDEX_OUTBOUND;
extern const uint16_t ParserState_ERR_UNKNOWN;

#endif
