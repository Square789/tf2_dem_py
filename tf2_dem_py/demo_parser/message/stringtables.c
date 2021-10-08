#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

#include "tf2_dem_py/demo_parser/message/stringtables.h"

void StringTableCreate_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	StringTableCreate_skip(caw, parser_state);
}

void StringTableCreate_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	uint16_t max_ln = CharArrayWrapper_get_uint16(caw);
	uint16_t max_ln_skip = ((uint16_t)log2(max_ln)) + 1;
	CharArrayWrapper_skip(caw, max_ln_skip / 8, max_ln_skip % 8);
	uint32_t len = CharArrayWrapper_get_var_int(caw);
	if (CharArrayWrapper_get_bit(caw) == 1) {
		CharArrayWrapper_skip(caw, 2, 0);
	}
	CharArrayWrapper_skip(caw, 0, 1);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}


void StringTableUpdate_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	StringTableUpdate_skip(caw, parser_state);
}

void StringTableUpdate_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 0, 5);
	if (CharArrayWrapper_get_bit(caw) == 1) {
		CharArrayWrapper_skip(caw, 2, 0);
	}
	uint32_t length = 0;
	CharArrayWrapper_read_raw(caw, &length, 2, 4);
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}
