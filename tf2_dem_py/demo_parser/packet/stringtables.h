#ifndef PACKET_STRINGTABLES__H
#define PACKET_STRINGTABLES__H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>

#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void Stringtables_parse(FILE *stream, ParserState *p_state);

#endif
