
#include <stdio.h> //for printf because who needs debuggers lol
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/stringtables.h"

uint32_t get_var_int(CharArrayWrapper *caw) {
	uint32_t res = 0;
	uint8_t i = 0;
	uint8_t read;
	for (i = 0; i < 35; i += 7) {
		read = CAW_get_uint8(caw);
		res |= ((read & 0x7F) << i);
		if ((read >> 7) == 0) {
			break;
		}
	}
	return res;
}

void p_StringTableCreate(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	printf("@%u :: ", caw->bytepos);
	s_StringTableCreate(caw, parser_state);
}

void s_StringTableCreate(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, CAW_dist_until_null(caw), 0);
	uint16_t max_ln = CAW_get_uint16(caw);
	uint16_t max_ln_skip = ((uint16_t)log2(max_ln)) + 1;
	CAW_skip(caw, max_ln_skip / 8, max_ln_skip % 8);
	uint32_t len = get_var_int(caw);
	printf("%u; %u; %u\n", max_ln, max_ln_skip, len);
	if (CAW_get_bit(caw) == 1) {
		CAW_skip(caw, 2, 0);
	}
	CAW_skip(caw, 0, 1);
	CAW_skip(caw, len / 8, len % 8);
}
