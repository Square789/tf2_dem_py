
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/packetentities.h"

void p_PacketEntities(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_PacketEntities(caw, parser_state);
}

void s_PacketEntities(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 3);
	if (CAW_get_bit(caw) == 1) {
		CAW_skip(caw, 4, 0);
	}
	CAW_skip(caw, 1, 4);
	uint32_t length = 0;
	CAW_read_raw(caw, &length, 2, 4); //Strangely, no shift required despite 3 bytes read into 4 bytes
	CAW_skip(caw, 0, 1);
	CAW_skip(caw, length / 8, length % 8);
}
