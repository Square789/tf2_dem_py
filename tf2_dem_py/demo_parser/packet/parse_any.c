#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <stdio.h>
#include <stdint.h>

#include "tf2_dem_py/demo_parser/packet/message.h"
#include "tf2_dem_py/demo_parser/packet/datatables.h"
#include "tf2_dem_py/demo_parser/packet/stringtables.h"
#include "tf2_dem_py/demo_parser/packet/synctick.h"
#include "tf2_dem_py/demo_parser/packet/consolecmd.h"
#include "tf2_dem_py/demo_parser/packet/usercmd.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

//Read the next byte from stream and determine what packet to read from this id.
// 
// FILE *stream : Pointer to a FILE object, next byte must be valid packet id
// ParserState *parser_state : Pointer to a ParserState_c instance, may be modified by this function.
// PyObject *root_dict : Pointer to a python dict, packet parser result will be written there.
void packet_parse_any(FILE *stream, ParserState *parser_state) {

	uint8_t packet_type;
	fread(&packet_type, sizeof(packet_type), 1, stream);

	//printf("Next packet type: %u @%u\n", packet_type, ftell(stream));
	switch (packet_type) {
	case 1:
		Message_parse(stream, parser_state); break;
	case 2:
		Message_parse(stream, parser_state); break;
	case 3:
		Synctick_parse(stream, parser_state); break;
	case 4:
		Consolecmd_parse(stream, parser_state); break;
	case 5:
		Usercmd_parse(stream, parser_state); break;
	case 6:
		Datatables_parse(stream, parser_state); break;
	case 7:
		printf("STOP\n");
		parser_state->finished = 1;
		break;
	case 8:
		Stringtables_parse(stream, parser_state); break;
	default:
		parser_state->failure |= ParserState_ERR_UNKNOWN_PACKET_ID;
	}
}
