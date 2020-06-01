
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/parsing/packet/message.hpp"
#include "tf2_dem_py/parsing/packet/datatables.hpp"
#include "tf2_dem_py/parsing/packet/stringtables.hpp"
#include "tf2_dem_py/parsing/packet/synctick.hpp"
#include "tf2_dem_py/parsing/packet/consolecmd.hpp"
#include "tf2_dem_py/parsing/packet/usercmd.hpp"

#include "tf2_dem_py/parsing/parser_state/parser_state.h"

void packet_parse_any(FILE *stream, ParserState *parser_state, PyObject *root_dict) {
	/*
	Read the next byte from stream and determine what packet to read from this id.

	FILE *stream : Pointer to a FILE object, next byte must be valid packet id
	ParserState *parser_state : Pointer to a ParserState, may be modified by this function.
	PyObject *root_dict : Pointer to a python dict, packet parser result will be written there.
	*/
	uint8_t packet_type;
	fread(&packet_type, sizeof(packet_type), 1, stream);

	printf("Next packet type: %u @%u\n", packet_type, ftell(stream));
	switch (packet_type)
	{
	case 1:
		Message_parse(stream, parser_state, root_dict); break;
	case 2:
		Message_parse(stream, parser_state, root_dict); break;
	case 3:
		Synctick_parse(stream, parser_state, root_dict); break;
	case 4:
		Consolecmd_parse(stream, parser_state, root_dict); break;
	case 5:
		Usercmd_parse(stream, parser_state, root_dict); break;
	case 6:
		Datatables_parse(stream, parser_state, root_dict); break;
	case 7:
		printf("STOP\n");
		parser_state->finished = 1;
	case 8:
		Stringtables_parse(stream, parser_state, root_dict); break;
	default:
		parser_state->FAILURE |= ParserState_ERR.UNKNOWN_PACKET_ID;
	}
}