
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/packetentities.hpp"

namespace MessageParsers {

class PacketEntities {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(1, 3);
		if (caw->get_bit() == 1) {
			caw->skip(4, 0);
		}
		caw->skip(1, 4);
		uint32_t length = 0;
		caw->read_raw(&length, 2, 4); //Strangely, no shift required despite 3 bytes read into 4 bytes
		caw->skip(0, 1);
		caw->skip(length / 8, length % 8);
	}
};

class TempEntities {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(1, 0);
		uint32_t length = caw->get_var_int();
		caw->skip(length / 8, length % 8);
	}
};

}