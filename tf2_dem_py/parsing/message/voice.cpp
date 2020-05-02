
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/voice.hpp"

namespace MessageParsers {

class ParseSounds {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		uint8_t reliable = caw->get_bit();
		uint8_t num;
		uint16_t length;
		if (reliable == 1) {
			num = 1;
			length = (uint16_t)caw->get_uint8();
		} else {
			num = caw->get_uint8();
			length = caw->get_uint16();
		}
		caw->skip(length / 8, length % 8);
	}
};

class VoiceInit {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(caw->dist_until_null(), 0);
		if (caw->get_uint8() == 255) {
			caw->skip(2, 0);
		}
	}
};

class VoiceData {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		uint16_t len = 0;
		caw->skip(2, 0);
		caw->read_raw(&len, 2, 0);
		caw->skip(len / 8, len % 8);
	}
};

}
