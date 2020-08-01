#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/packet/consolecmd.hpp"

using ParserState::ParserState_c;

void Synctick_parse(FILE *stream, ParserState_c *p_state, PyObject *root_dict) {
	uint32_t tick;

	// That is all
	fread(&tick, sizeof(tick), 1, stream);
}
