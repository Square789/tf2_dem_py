
#include <math.h>
#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/__init__.hpp"
#include "tf2_dem_py/parsing/message/classinfo.h"

namespace MessageParsers {

void ClassInfo::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void ClassInfo::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t count = caw->get_uint16();
	uint16_t what = (uint16_t)log2(count);
	uint8_t create = caw->get_bit();
	if (create == 0) {
		for (int i = 0; i < count; i++) {
			caw->skip(what / 8, what % 8);
			caw->skip(caw->dist_until_null(), 0);
			caw->skip(caw->dist_until_null(), 0);
		}
	}
}

}
