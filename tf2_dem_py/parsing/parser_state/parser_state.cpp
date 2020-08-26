
#include <stdint.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

namespace ParserState {

namespace ERRORS {

const uint16_t CAW = 1 << 0;
const uint16_t UNKNOWN_PACKET_ID = 1 << 1;
const uint16_t IO = 1 << 2;
const uint16_t UNEXPECTED_EOF = 1 << 3;
const uint16_t UNKNOWN_MESSAGE_ID = 1 << 4;
const uint16_t MEMORY_ALLOCATION = 1 << 5;
const uint16_t UNKNOWN_GAME_EVENT = 1 << 6;
const uint16_t PYDICT = 1 << 7;
const uint16_t PYLIST = 1 << 8;
const uint16_t GAME_EVENT_INDEX_OUTBOUND = 1 << 9;
const uint16_t UNKNOWN = 1 << 15;
	
}

}
