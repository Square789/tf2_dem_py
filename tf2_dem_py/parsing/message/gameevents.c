
#include <math.h>
#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/gameevents.h"

void p_GameEvent(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_GameEvent(caw, parser_state);
}

void s_GameEvent(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}


void p_GameEventList(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_GameEventList(caw, parser_state);
}

void s_GameEventList(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 1);
	uint32_t len = 0;
	CAW_read_raw(caw, &len, 2, 4);
	//len >>= 8; // Shift unneccessary, why-ever.
	CAW_skip(caw, len / 8, len % 8);
}
