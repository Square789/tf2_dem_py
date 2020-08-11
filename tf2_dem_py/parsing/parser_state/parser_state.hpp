#ifndef PARSER_STATE__HPP
#define PARSER_STATE__HPP

#include <stdint.h>
#include "tf2_dem_py/parsing/game_events/game_events.hpp"

#include <vector>

namespace ParserState {

struct ParserState_c {
	uint8_t current_message;
	uint32_t flags;
	uint8_t finished;
	uint8_t FAILURE;
	uint8_t RELAYED_CAW_ERR;
	uint32_t tick;
	uint32_t game_event_def_amount;
	GameEvents::GameEventDefinition *game_event_defs;

	ParserState_c():
		current_message(255),
		flags(0),
		finished(0),
		FAILURE(0),
		RELAYED_CAW_ERR(0),
		tick(0),
		game_event_def_amount(0),
		game_event_defs(NULL)
		{}

	~ParserState_c() {
		if (game_event_defs != NULL) {
			delete[] game_event_defs;
		}
	}
};

namespace ERRORS {
	extern const uint16_t CAW;
	extern const uint16_t UNKNOWN_PACKET_ID;
	extern const uint16_t IO;
	extern const uint16_t UNEXPECTED_EOF;
	extern const uint16_t UNKNOWN_MESSAGE_ID;
	extern const uint16_t MEMORY_ALLOCATION;
	extern const uint16_t UNKNOWN_GAME_EVENT;
	extern const uint16_t PYDICT;
	extern const uint16_t PYLIST;
	extern const uint16_t UNKNOWN;
}

}

#endif
