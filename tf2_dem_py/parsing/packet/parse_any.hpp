
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.h"

void packet_parse_any(FILE *stream, ParserState *p_state, PyObject *root_json);
