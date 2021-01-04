#ifndef PACKET_CONSOLECMD__H
#define PACKET_CONSOLECMD__H

#include <stdio.h>

#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void Consolecmd_parse(FILE *stream, ParserState *p_state);

#endif
