from libc.stdio cimport FILE, fread, ftell, printf
from libc.stdint cimport uint8_t
from libc.string cimport memset

cimport tf2_dem_py.parsing.packet.message as message
cimport tf2_dem_py.parsing.packet.datatables as datatables
cimport tf2_dem_py.parsing.packet.stringtables as stringtables
cimport tf2_dem_py.parsing.packet.synctick as synctick
cimport tf2_dem_py.parsing.packet.consolecmd as consolecmd
cimport tf2_dem_py.parsing.packet.usercmd as usercmd
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef dict parse_any(FILE *stream, ParserState *parser_state):
	cdef uint8_t packet_type
	fread(&packet_type, sizeof(packet_type), 1, stream)

	#printf("Next packet at %16u: %u\n", <int>ftell(stream), <int>packet_type)

	if packet_type == 1:
		message.parse(stream, parser_state)
	elif packet_type == 2:
		message.parse(stream, parser_state)
	elif packet_type == 3:
		synctick.parse(stream, parser_state)
	elif packet_type == 4:
		consolecmd.parse(stream, parser_state)
	elif packet_type == 5:
		usercmd.parse(stream, parser_state)
	elif packet_type == 6:
		datatables.parse(stream, parser_state)
	elif packet_type == 7:
		printf("STOP\n")
		parser_state.finished = 1
	elif packet_type == 8:
		stringtables.parse(stream, parser_state)
	else:
		parser_state.finished = 1
		parser_state.FAILURE |= 0b10

	return {}
