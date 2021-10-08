#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

#include "tf2_dem_py/demo_parser/message/packetentities.h"

void PacketEntities_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	PacketEntities_skip(caw, parser_state);
}

void PacketEntities_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint32_t length = 0;
	CharArrayWrapper_skip(caw, 1, 3);
	if (CharArrayWrapper_get_bit(caw) == 1) {
		CharArrayWrapper_skip(caw, 4, 0);
	}
	CharArrayWrapper_skip(caw, 1, 4);
	CharArrayWrapper_read_raw(caw, &length, 2, 4); //Strangely, no shift required despite 3 bytes read into 4 bytes
	CharArrayWrapper_skip(caw, 0, 1);
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}


void TempEntities_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	TempEntities_skip(caw, parser_state);
}

void TempEntities_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 0);
	uint32_t length = CharArrayWrapper_get_var_int(caw);
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}
