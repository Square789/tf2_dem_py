#ifndef PARSER_STATE__H
#define PARSER_STATE__H

#include <stdint.h>
#include "tf2_dem_py/parsing/game_events/game_events.hpp"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ParserState {
	uint32_t flags;
	uint8_t finished;
	uint8_t FAILURE;
	uint8_t RELAYED_CAW_ERR;
	uint32_t tick;
	GameEventDefinitionArray *game_event_defs;
	uint8_t current_message_contains_senderless_chat;
} ParserState;

typedef struct ERR_s {
	uint16_t CAW;
	uint16_t UNKNOWN_PACKET_ID;
	uint16_t IO;
	uint16_t UNEXPECTED_EOF;
	uint16_t UNKNOWN_MESSAGE_ID;
	uint16_t MEMORY_ALLOCATION;
	uint16_t UNKNOWN_GAME_EVENT;
	uint16_t PYDICT;

	uint16_t UNKNOWN;
} ERR_s;

extern const ERR_s ParserState_ERR;

#ifdef __cplusplus
}
#endif

#endif
