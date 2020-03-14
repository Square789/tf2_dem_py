
#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

void p_Empty(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	}

void s_Empty(CharArrayWrapper *caw, ParserState *parser_state) {
	}

void p_Print(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint8_t *str_ptr = CAW_get_nulltrm_str(caw);

	if (CAW_get_errorlevel(caw) != 0) {
		parser_state->FAILURE |= 0b1;
		parser_state->RELAYED_CAW_ERR = CAW_get_errorlevel(caw);
		return;
	}

	if (cJSON_AddVolatileStringRefToObject(root_json, "printmsglol", (const char *)str_ptr) == NULL) {
		parser_state->FAILURE |= 0b10000;
		return;
	}
}

void s_Print(CharArrayWrapper *caw, ParserState *parser_state) {
	size_t dist = CAW_dist_until_null(caw);
	CAW_skip(caw, dist, 0); // no error checks, occurs in message parsing loop
}

void p_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	cJSON *sinfo_json = cJSON_AddObjectToObject(root_json, "serverinfo");
	uint8_t json_err = 0;

	if (sinfo_json == NULL) {
		parser_state->FAILURE |= 0b10000;
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
		parser_state->FAILURE |= 0b1;
		parser_state->RELAYED_CAW_ERR = CAW_get_errorlevel(caw);
		return;
	}

	if (json_err == 1) {
		parser_state->FAILURE |= 0b10000;
		return;
	}
}

void s_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 36, 2);
	CAW_skip(caw, CAW_dist_until_null(caw), 0);
	CAW_skip(caw, CAW_dist_until_null(caw), 0);
	CAW_skip(caw, CAW_dist_until_null(caw), 0);
	CAW_skip(caw, CAW_dist_until_null(caw), 0); // Less overhead than a loop i guess
	CAW_skip(caw, 0, 1);
}
