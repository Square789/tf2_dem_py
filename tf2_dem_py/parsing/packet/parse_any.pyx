from libc.stdio cimport FILE, fread, ftell, printf
from libc.stdint cimport uint8_t

cimport tf2_dem_py.parsing.packet.message as message
cimport tf2_dem_py.parsing.packet.datatables as datatables
cimport tf2_dem_py.parsing.packet.stringtables as stringtables
cimport tf2_dem_py.parsing.packet.synctick as synctick
cimport tf2_dem_py.parsing.packet.consolecmd as consolecmd
cimport tf2_dem_py.parsing.packet.usercmd as usercmd
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef void parse_any(FILE *stream, ParserState *parser_state, cJSON *root_json):
	"""
	Read the next byte from stream and determine what packet to read from this id.

	FILE *stream : Pointer to a FILE object, next byte must be valid packet id
	ParserState *parser_state : Pointer to a ParserState, may be modified by this function.
	cJSON *root_json : Pointer to a cJSON object, packet parser result will be written there.
	"""
	cdef uint8_t packet_type
	fread(&packet_type, sizeof(packet_type), 1, stream)

	if packet_type == 1:
		message.parse(stream, parser_state, root_json)
	elif packet_type == 2:
		message.parse(stream, parser_state, root_json)
	elif packet_type == 3:
		synctick.parse(stream, parser_state, root_json)
	elif packet_type == 4:
		consolecmd.parse(stream, parser_state, root_json)
	elif packet_type == 5:
		usercmd.parse(stream, parser_state, root_json)
	elif packet_type == 6:
		datatables.parse(stream, parser_state, root_json)
	elif packet_type == 7:
		printf("STOP\n")
		parser_state.finished = 1
	elif packet_type == 8:
		stringtables.parse(stream, parser_state, root_json)
	else:
		parser_state.FAILURE |= 0b10
