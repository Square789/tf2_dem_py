
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/stringtables.hpp"

namespace MessageParsers {

class StringTableCreate {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		// uint8_t *str = CAW_get_nulltrm_str(caw);
		// if (str != 0) {
		// 	printf("%s\n", str);
		// } else { printf("huh\n"); }
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
		uint16_t max_ln = CAW_get_uint16(caw);
		uint16_t max_ln_skip = ((uint16_t)log2(max_ln)) + 1;
		CAW_skip(caw, max_ln_skip / 8, max_ln_skip % 8);
		uint32_t len = CAW_get_var_int(caw);
		if (CAW_get_bit(caw) == 1) {
			CAW_skip(caw, 2, 0);
		}
		CAW_skip(caw, 0, 1);
		CAW_skip(caw, len / 8, len % 8);
	}
};

class StringTableUpdate {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		// target_table = stream.read_int(5)
		// changed = stream.read_int(16) if stream.read_boolean() else 1
		// length = stream.read_int(20)
		// data = stream.read_raw(length)
		CAW_skip(caw, 0, 5);
		if (CAW_get_bit(caw) == 1) {
			CAW_skip(caw, 2, 0);
		}
		uint32_t length;
		CAW_read_raw(caw, &length, 2, 4);
		CAW_skip(caw, length / 8, length % 8);
	}
};

}