
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/stringtables.hpp"

namespace MessageParsers {

class StringTableCreate {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		// uint8_t *str = caw->get_nulltrm_str();
		// if (str != 0) {
		// 	printf("%s\n", str);
		// } else { printf("huh\n"); }
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(caw->dist_until_null(), 0);
		uint16_t max_ln = caw->get_uint16();
		uint16_t max_ln_skip = ((uint16_t)log2(max_ln)) + 1;
		caw->skip(max_ln_skip / 8, max_ln_skip % 8);
		uint32_t len = caw->get_var_int();
		if (caw->get_bit() == 1) {
			caw->skip(2, 0);
		}
		caw->skip(0, 1);
		caw->skip(len / 8, len % 8);
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
		caw->skip(0, 5);
		if (caw->get_bit() == 1) {
			caw->skip(2, 0);
		}
		uint32_t length;
		caw->read_raw(&length, 2, 4);
		caw->skip(length / 8, length % 8);
	}
};

}