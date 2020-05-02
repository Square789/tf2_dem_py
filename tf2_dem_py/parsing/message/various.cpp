
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
		CAW_skip(caw, 4, 0);
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
		CAW_skip(caw, 0, 1);
	}
};

class NetTick {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		uint32_t tick = CAW_get_uint32(caw);
		uint16_t frame_time = CAW_get_uint16(caw);
		uint16_t std_dev = CAW_get_uint16(caw);

		parser_state->tick = tick;
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 8, 0);
	}
};

class StringCommand {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
	}
};

class SetConVar {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		uint8_t amt = CAW_get_uint8(caw);
		for (uint16_t i = 0; i < amt; i++) {
			CAW_skip(caw, CAW_dist_until_null(caw), 0);
			CAW_skip(caw, CAW_dist_until_null(caw), 0);
		}
	}
};

class SigOnState {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 5, 0);
	}
};

class Print {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		uint8_t *str_ptr = CAW_get_nulltrm_str(caw);

		if (CAW_get_errorlevel(caw) != 0) {
			parser_state->FAILURE |= ERR.CAW;
			parser_state->RELAYED_CAW_ERR = CAW_get_errorlevel(caw);
			return;
		}

		if (cJSON_AddVolatileStringRefToObject(root_json, "printmsglol", (const char *)str_ptr) == NULL) {
			parser_state->FAILURE |= ERR.CJSON;
			return;
		}
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		size_t dist = CAW_dist_until_null(caw);
		CAW_skip(caw, dist, 0); // no error checks, occurs in message parsing loop
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

		if (cJSON_AddNumberToObject(sinfo_json, "version", CAW_get_uint16(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "server_count", CAW_get_uint32(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "stv", CAW_get_bit(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "dedicated", CAW_get_bit(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_crc", CAW_get_uint32(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_classes", CAW_get_uint16(caw)) == NULL) { json_err = 1; }
		CAW_skip(caw, 16, 0);
		// if cJSON_AddVolatileStringRefToObject(sinfo_json, "map_hash", (const char *)CAW_get_chars(caw, 16)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "player_count", CAW_get_uint8(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "max_player_count", CAW_get_uint8(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "interval_per_tick", CAW_get_flt(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddNumberToObject(sinfo_json, "platform", CAW_get_uint8(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "game", (const char *)CAW_get_nulltrm_str(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "map_name", (const char *)CAW_get_nulltrm_str(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "skybox", (const char *)CAW_get_nulltrm_str(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddVolatileStringRefToObject(sinfo_json, "server_name", (const char *)CAW_get_nulltrm_str(caw)) == NULL) { json_err = 1; }
		if (cJSON_AddBoolToObject(sinfo_json, "replay", CAW_get_bit(caw)) == NULL) { json_err = 1; }

		if (CAW_get_errorlevel(caw) != 0) {
			parser_state->FAILURE |= ERR.CAW;
			parser_state->RELAYED_CAW_ERR = CAW_get_errorlevel(caw);
			return;
		}

		if (json_err == 1) {
			parser_state->FAILURE |= ERR.CJSON;
			return;
		}
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 36, 2);
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
		CAW_skip(caw, CAW_dist_until_null(caw), 0); // Less overhead than a loop i guess
		CAW_skip(caw, 0, 1);
	}
};

class SetView {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 1, 3);
	}
};

class FixAngle {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 6, 1);
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
			existing_coords[i] = CAW_get_bit(caw);
		}
		for (i = 0; i < 3; i++) {
			if (existing_coords[i] == 1) {
				CAW_get_bit_coord(caw);
			} 
		}
		CAW_skip(caw, 1, 1);
		if (CAW_get_bit(caw) == 1) {
			CAW_skip(caw, 3, 0);
		}
		CAW_skip(caw, 0, 1);
	}
};

class Entity {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 2, 4);
		uint16_t length;
		CAW_read_raw(caw, &length, 1, 3);
		CAW_skip(caw, length / 8, length % 8);
	}
};

class PreFetch {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 1, 6);
	}
};

class GetCvarValue {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
		this->skip(caw, parser_state);
	}

	void skip(CharArrayWrapper *caw, ParserState *parser_state) {
		CAW_skip(caw, 4, 0);
		CAW_skip(caw, CAW_dist_until_null(caw), 0);
	}
};

}
