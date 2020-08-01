#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/packet/consolecmd.hpp"

using ParserState::ParserState_c;

void Usercmd_parse(FILE *stream, ParserState_c *p_state, PyObject *root_dict) {
	uint32_t tick;
	uint32_t pkt_len;

	// Read tick of packet data
	fread(&tick, sizeof(tick), 1, stream);

	fseek(stream, 4, SEEK_CUR); // Skip "sequence_out"

	// Read length of packet data
	fread(&pkt_len, sizeof(pkt_len), 1, stream);

	if (ferror(stream) != 0) {
		p_state->FAILURE |= ParserState::ERRORS::IO;
		return;
	}
	if (feof(stream) != 0) {
		p_state->FAILURE |= ParserState::ERRORS::UNEXPECTED_EOF;
		return;
	}

	// Skip the thing lol
	fseek(stream, pkt_len, SEEK_CUR);
}
