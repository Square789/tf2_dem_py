#ifndef PARSER_STATE__H
#define PARSER_STATE__H

#include <stdint.h>
#include "tf2_dem_py/parsing/game_events/game_events.h"

typedef struct ParserState {
	uint16_t flags;
	uint8_t finished;
	uint8_t FAILURE;
	uint8_t RELAYED_CAW_ERR;
	uint32_t tick;
	GameEventDefinitionArray *game_event_defs;
} ParserState;

typedef struct ERR_s {
	uint8_t CAW;
	uint8_t UNKNOWN_PACKET_ID;
	uint8_t IO;
	uint8_t UNEXPECTED_EOF;
	uint8_t CJSON;
	uint8_t UNKNOWN_MESSAGE_ID;
	uint8_t MEMORY_ALLOCATION;
} ERR_s;

extern const ERR_s ERR;

#endif
