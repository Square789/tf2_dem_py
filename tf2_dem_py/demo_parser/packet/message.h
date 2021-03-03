#ifndef PACKET_MESSAGE__H
#define PACKET_MESSAGE__H

#include <stdio.h>

#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void Message_parse(FILE *stream, ParserState *p_state);

#endif
