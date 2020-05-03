
#include <stdio.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/cJSON/cJSON.h"


void parse_demo_header(FILE *stream, ParserState* p_state, cJSON *root_json) {
	char json_err = 0;

	CharArrayWrapper *header_caw = CAW_from_file(stream, 1072);
	cJSON *header = cJSON_AddObjectToObject(root_json, "header");

	// Implicit type casts to double in AddNumber
	if (cJSON_AddVolatileStringRefToObject(header, "ident", (const char *)header_caw->get_chars(8)) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "net_prot", header_caw->get_uint32()) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "dem_prot", header_caw->get_uint32()) == NULL) json_err = 1;
	if (cJSON_AddVolatileStringRefToObject(header, "host_addr", (const char *)header_caw->get_chars(260)) == NULL) json_err = 1;
	if (cJSON_AddVolatileStringRefToObject(header, "client_id", (const char *)header_caw->get_chars(260)) == NULL) json_err = 1;
	if (cJSON_AddVolatileStringRefToObject(header, "map_name", (const char *)header_caw->get_chars(260)) == NULL) json_err = 1;
	if (cJSON_AddVolatileStringRefToObject(header, "game_dir", (const char *)header_caw->get_chars(260)) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "play_time", header_caw->get_flt()) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "tick_count", header_caw->get_uint32()) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "frame_count", header_caw->get_uint32()) == NULL) json_err = 1;
	if (cJSON_AddNumberToObject(header, "sigon", header_caw->get_uint32()) == NULL) json_err = 1;

	if (header_caw->ERRORLEVEL != 0) {
		p_state->FAILURE |= ERR.CAW;
		p_state->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
	}

	if (json_err == 1) {
		p_state->FAILURE |= ERR.CJSON;
	}
}
