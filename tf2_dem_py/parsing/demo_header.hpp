#ifndef DEMO_HEADER__HPP
#define DEMO_HEADER__HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

using ParserState::ParserState_c;

void parse_demo_header(FILE *stream, ParserState_c *p_state, PyObject *root_dict);

#endif
