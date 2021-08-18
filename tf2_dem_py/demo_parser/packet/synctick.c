#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"
#include "tf2_dem_py/demo_parser/packet/synctick.h"

void Synctick_parse(FILE *stream, ParserState *p_state) {
	uint32_t tick;

	// That is all
	fread(&tick, sizeof(tick), 1, stream);
}
