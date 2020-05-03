
#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/various.hpp"

namespace MessageParsers {

void Empty::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {}
void Empty::skip(CharArrayWrapper *caw, ParserState *parser_state) {}


void File::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void File::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(4, 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(0, 1);
}


void NetTick::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint32_t tick = caw->get_uint32();
	uint16_t frame_time = caw->get_uint16();
	uint16_t std_dev = caw->get_uint16();

	parser_state->tick = tick;
}

void NetTick::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(8, 0);
}


void StringCommand::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void StringCommand::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(caw->dist_until_null(), 0);
}


void SetConVar::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void SetConVar::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t amt = caw->get_uint8();
	for (uint16_t i = 0; i < amt; i++) {
		caw->skip(caw->dist_until_null(), 0);
		caw->skip(caw->dist_until_null(), 0);
	}
}


void SigOnState::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void SigOnState::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(5, 0);
}


void Print::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	const char *str_ptr = caw->get_nulltrm_str();

	if (caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ERR.CAW;
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		return;
	}

	if (cJSON_AddVolatileStringRefToObject(root_json, "printmsglol", str_ptr) == NULL) {
		parser_state->FAILURE |= ERR.CJSON;
		return;
	}
}

void Print::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	size_t dist = caw->dist_until_null();
	caw->skip(dist, 0); // no error checks, occurs in message parsing loop
}


void ServerInfo::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
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

	if (caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ERR.CAW;
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		return;
	}

	if (json_err == 1) {
		parser_state->FAILURE |= ERR.CJSON;
		return;
	}
}

void ServerInfo::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(36, 2);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0); // Less overhead than a loop i guess
	caw->skip(0, 1);
}


void SetView::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void SetView::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 3);
}


void FixAngle::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void FixAngle::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(6, 1);
}


void BspDecal::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void BspDecal::skip(CharArrayWrapper *caw, ParserState *parser_state) {
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


void Entity::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void Entity::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(2, 4);
	uint16_t length;
	caw->read_raw(&length, 1, 3);
	caw->skip(length / 8, length % 8);
}


void PreFetch::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void PreFetch::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 6);
}


void GetCvarValue::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	this->skip(caw, parser_state);
}

void GetCvarValue::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(4, 0);
	caw->skip(caw->dist_until_null(), 0);
}

}
