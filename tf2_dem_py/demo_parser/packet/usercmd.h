#ifndef PACKET_USERCMD__H
#define PACKET_USERCMD__H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>

#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void Usercmd_parse(FILE *stream, ParserState *p_state);

#endif
