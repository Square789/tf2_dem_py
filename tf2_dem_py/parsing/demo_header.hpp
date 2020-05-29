#ifndef DEMO_HEADER__HPP
#define DEMO_HEADER__HPP

#include <stdio.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/cJSON/cJSON.h"

void parse_demo_header(FILE *stream, ParserState *p_state, cJSON *root_json);

#endif
