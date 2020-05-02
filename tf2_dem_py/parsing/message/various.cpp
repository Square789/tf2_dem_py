
#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/various.hpp"

namespace MessageParsers {

class Empty {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {}
	void skip(CharArrayWrapper *caw, ParserState *parser_state) {}
};

class File {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(4, 0);
		caw->skip(caw->dist_until_null(), 0);
		caw->skip(0, 1);
	}
};

class NetTick {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		uint32_t tick = caw->get_uint32();
		uint16_t frame_time = caw->get_uint16();
		uint16_t std_dev = caw->get_uint16();

		parser_state->tick = tick;
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(8, 0);
	}
};

class StringCommand {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(caw->dist_until_null(), 0);
	}
};

class SetConVar {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		uint8_t amt = caw->get_uint8();
		for (uint16_t i = 0; i < amt; i++) {
			caw->skip(caw->dist_until_null(), 0);
			caw->skip(caw->dist_until_null(), 0);
		}
	}
};

class SigOnState {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(5, 0);
	}
};

class Print {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		const char *str_ptr = caw->get_nulltrm_str();

		if (caw->get_errorlevel() != 0) {
			parser_state->FAILURE |= ERR.CAW;
			parser_state->RELAYED_CAW_ERR = caw->get_errorlevel();
			return;
		}

		if (cJSON_AddVolatileStringRefToObject(root_json, "printmsglol", str_ptr) == NULL) {
			parser_state->FAILURE |= ERR.CJSON;
			return;
		}
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		size_t dist = caw->dist_until_null();
		caw->skip(dist, 0); // no error checks, occurs in message parsing loop
	}
};

class ServerInfo {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		cJSON *sinfo_json = cJSON_AddObjectToObject(root_json, "serverinfo");
		uint8_t json_err = 0;

		if (sinfo_json == NULL) {
			parser_state->FAILURE |= ERR.CJSON;
			return;
		}

		if (cJSON_AddNumberToObject(sinfo_json, "version", caw->get_uint16()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "server_count", caw->get_uint32()) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "stv", caw->get_bit()) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "dedicated", caw->get_bit()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_crc", caw->get_uint32()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_classes", caw->get_uint16()) == NULL) { json_err = 1; }
		caw->skip(16, 0);
		// if cJSON_AddVolatileStringRefToObject(sinfo_json, "map_hash", (const char *)caw->get_chars(16)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "player_count", caw->get_uint8()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_player_count", caw->get_uint8()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "interval_per_tick", caw->get_flt()) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "platform", caw->get_uint8()) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "game", (const char *)caw->get_nulltrm_str()) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "map_name", (const char *)caw->get_nulltrm_str()) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "skybox", (const char *)caw->get_nulltrm_str()) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "server_name", (const char *)caw->get_nulltrm_str()) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "replay", caw->get_bit()) == NULL) { json_err = 1; }

		if (caw->get_errorlevel() != 0) {
			parser_state->FAILURE |= ERR.CAW;
			parser_state->RELAYED_CAW_ERR = caw->get_errorlevel();
			return;
		}

		if (json_err == 1) {
			parser_state->FAILURE |= ERR.CJSON;
			return;
		}
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(36, 2);
		caw->skip(caw->dist_until_null(), 0);
		caw->skip(caw->dist_until_null(), 0);
		caw->skip(caw->dist_until_null(), 0);
		caw->skip(caw->dist_until_null(), 0); // Less overhead than a loop i guess
		caw->skip(0, 1);
	}
};

class SetView {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(1, 3);
	}
};

class FixAngle {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(6, 1);
	}
};

class BspDecal {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		uint8_t existing_coords[3] = {0, 0, 0};
		uint8_t i;
		for (i = 0; i < 3; i++) {
			existing_coords[i] = caw->get_bit();
		}
		for (i = 0; i < 3; i++) {
			if (existing_coords[i] == 1) {
				caw->get_bit_coord();
			} 
		}
		caw->skip(1, 1);
		if (caw->get_bit() == 1) {
			caw->skip(3, 0);
		}
		caw->skip(0, 1);
	}
};

class Entity {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(2, 4);
		uint16_t length;
		caw->read_raw(&length, 1, 3);
		caw->skip(length / 8, length % 8);
	}
};

class PreFetch {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(1, 6);
	}
};

class GetCvarValue {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		caw->skip(4, 0);
		caw->skip(caw->dist_until_null(), 0);
	}
};

}
