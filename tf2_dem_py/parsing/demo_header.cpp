#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "tf2_dem_py/helpers.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

void parse_demo_header(FILE *stream, ParserState* p_state, PyObject *root_dict) {
	static char *HEADER_NAMES[] = {
		"ident", "net_prot", "dem_prot", "host_addr",
		"client_id", "map_name", "game_dir", "play_time",
		"tick_count", "frame_count", "sigon",
	};

	CharArrayWrapper *header_caw = CAW_from_file(stream, 1072);
	if (header_caw->ERRORLEVEL != 0) {
		p_state->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
		p_state->FAILURE |= ParserState_ERR.CAW;
		return;
	}
	PyObject *header_dict = PyDict_New();
	if (header_dict == NULL) {
		p_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}

	// Load header values into header, already converted to PyObjects
	PyObject *header[11];

	header[0]  = PyUnicode_FromCAWLen(header_caw, 8);
	header[1]  = PyLong_FromLong(header_caw->get_uint32());
	header[2]  = PyLong_FromLong(header_caw->get_uint32());
	header[3]  = PyUnicode_FromCAWLen(header_caw, 260);
	header[4]  = PyUnicode_FromCAWLen(header_caw, 260);
	header[5]  = PyUnicode_FromCAWLen(header_caw, 260);
	header[6]  = PyUnicode_FromCAWLen(header_caw, 260);
	header[7]  = PyFloat_FromDouble(header_caw->get_flt());
	header[8]  = PyLong_FromLong(header_caw->get_uint32());
	header[9]  = PyLong_FromLong(header_caw->get_uint32());
	header[10] = PyLong_FromLong(header_caw->get_uint32());

	for (uint8_t i = 0; i < 11; i++) {
		if (header[i] == NULL) { // Python conversion failure, error raised already
			p_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		} else {
			if (PyDict_SetItemString(header_dict, HEADER_NAMES[i], header[i]) < 0) {
				p_state->FAILURE |= ParserState_ERR.PYDICT;
			}
			Py_DECREF(header[i]);
		}
	}

	if (PyDict_SetItemString(root_dict, "header", header_dict) < 0) {
		p_state->FAILURE |= ParserState_ERR.PYDICT;
	}

	if (header_caw->ERRORLEVEL != 0) {
		p_state->FAILURE |= ParserState_ERR.CAW;
		p_state->RELAYED_CAW_ERR = header_caw->ERRORLEVEL;
	}
}
