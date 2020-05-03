#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/cJSON/cJSON.h"

#include "tf2_dem_py/parsing/packet/consolecmd.hpp"

void Synctick_parse(FILE *stream, ParserState *p_state, cJSON *root_json) {
	uint32_t tick;

	// That is all
	fread(&tick, sizeof(tick), 1, stream);
}
