
#include <math.h>
#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/classinfo.h"

void p_ClassInfo(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_ClassInfo(caw, parser_state);
}

void s_ClassInfo(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t count = CAW_get_uint16(caw);
	uint16_t what = (uint16_t)log2(count);
	uint8_t create = CAW_get_bit(caw);
	if (create == 0) {
		for (int i = 0; i < count; i++) {
			CAW_skip(caw, what / 8, what % 8);
			CAW_skip(caw, CAW_dist_until_null(caw), 0);
			CAW_skip(caw, CAW_dist_until_null(caw), 0);
		}
	}
}
