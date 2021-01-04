#include <math.h>
#include <stdint.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/message/__init__.h"
#include "tf2_dem_py/demo_parser/message/classinfo.h"

void ClassInfo_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	ClassInfo_skip(caw, parser_state);
}

void ClassInfo_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t count = CharArrayWrapper_get_uint16(caw);
	uint16_t what = (uint16_t)log2(count);
	uint8_t create = CharArrayWrapper_get_bit(caw);
	if (create == 0) {
		for (int i = 0; i < count; i++) {
			CharArrayWrapper_skip(caw, what / 8, what % 8);
			CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
			CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
		}
	}
}
