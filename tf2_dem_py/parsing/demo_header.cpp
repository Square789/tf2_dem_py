#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "tf2_dem_py/parsing/helpers.hpp"
#include "tf2_dem_py/constants.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

using ParserState::ParserState_c;

void parse_demo_header(FILE *stream, ParserState_c *p_state, PyObject *root_dict) {
	CharArrayWrapper *header_caw = caw_from_file(stream, 1072);
	if (header_caw->ERRORLEVEL != 0) {
		p_state->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
		p_state->FAILURE |= ParserState::ERRORS::CAW;
		return;
	}
	PyObject *header_dict = PyDict_New();
	if (header_dict == NULL) {
		p_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	// Load header values into header, already converted to PyObjects
	PyObject *header[11];

	header[0]  = PyUnicode_FromCAWLen(header_caw, 8);
	header[1]  = PyLong_FromLong(header_caw->get_uint32());
	header[2]  = PyLong_FromLong(header_caw->get_uint32());
	header[3]  = PyUnicode_FromCAWUpToNull(header_caw, 260);
	header[4]  = PyUnicode_FromCAWUpToNull(header_caw, 260);
	header[5]  = PyUnicode_FromCAWUpToNull(header_caw, 260);
	header[6]  = PyUnicode_FromCAWUpToNull(header_caw, 260);
	header[7]  = PyFloat_FromDouble(header_caw->get_flt());
	header[8]  = PyLong_FromLong(header_caw->get_uint32());
	header[9]  = PyLong_FromLong(header_caw->get_uint32());
	header[10] = PyLong_FromLong(header_caw->get_uint32());

	for (uint8_t i = 0; i < 11; i++) {
		if (header[i] == NULL) { // Python conversion failure, error raised already
			p_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		} else {
			if (PyDict_SetItem(header_dict, CONSTANTS::DICT_NAMES_header->py_strings[i], header[i]) < 0) {
				p_state->FAILURE |= ParserState::ERRORS::PYDICT;
			}
			Py_DECREF(header[i]);
		}
	}

	if (PyDict_SetItemString(root_dict, "header", header_dict) < 0) {
		p_state->FAILURE |= ParserState::ERRORS::PYDICT;
	}

	if (header_caw->ERRORLEVEL != 0) {
		p_state->FAILURE |= ParserState::ERRORS::CAW;
		p_state->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
	}
}
