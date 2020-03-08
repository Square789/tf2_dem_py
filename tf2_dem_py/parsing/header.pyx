from libc.stdio cimport FILE

from tf2_dem_py.parsing.chararray_wrapper cimport CharArrayWrapper
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef dict parse(FILE *stream, ParserState* p_state):
	cdef dict hdict = {}
	cdef CharArrayWrapper header_stream = \
		CharArrayWrapper.create_new(stream, 1072)

	hdict["ident"] = header_stream.get_utf8_str(8)
	hdict["net_prot"] = header_stream.get_uint32()
	hdict["dem_prot"] = header_stream.get_uint32()
	hdict["host_addr"] = header_stream.get_utf8_str(260)
	hdict["client_id"] = header_stream.get_utf8_str(260)
	hdict["map_name"] = header_stream.get_utf8_str(260)
	hdict["game_dir"] = header_stream.get_utf8_str(260)
	hdict["play_time"] = header_stream.get_dbl32()
	hdict["tick_count"] = header_stream.get_uint32()
	hdict["frame_count"] = header_stream.get_uint32()
	hdict["sigon"] = header_stream.get_uint32()

	if header_stream.ERRORLEVEL != 0:
		p_state.FAILURE |= 0b1
		p_state.RELAYED_CAW_ERR = header_stream.ERRORLEVEL
		return {}

	return hdict
