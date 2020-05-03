
#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/cJSON/cJSON.h"

#include "tf2_dem_py/parsing/packet/consolecmd.hpp"

void Consolecmd_parse(FILE *stream, ParserState *p_state, cJSON *root_json) {
	uint32_t tick;
	uint32_t pkt_len;

	// Read tick of packet data
	fread(&tick, sizeof(tick), 1, stream);

	// Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		p_state->FAILURE |= ERR.IO;
		return;
	}
	if (feof(stream) != 0) {
		p_state->FAILURE |= ERR.UNEXPECTED_EOF;
		return;
	}

	// Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR);
}
