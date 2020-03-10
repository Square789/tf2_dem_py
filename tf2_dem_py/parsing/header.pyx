from libc.stdio cimport FILE

from tf2_dem_py.parsing.chararray_wrapper cimport CharArrayWrapper
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON.cJSON_wrapper cimport (cJSON, cJSON_CreateObject,
	cJSON_AddNumberToObject, cJSON_AddStringToObject, cJSON_AddObjectToObject)

cdef void parse(FILE *stream, ParserState* p_state, cJSON *root_json):
	"""
	Parses the demo header from an open file stream.
	"""
	cdef char json_err = 0

	cdef CharArrayWrapper header_stream = \
		CharArrayWrapper.create_new(stream, 1072)
	cdef cJSON *header = cJSON_AddObjectToObject(root_json, "header")

	# Implicit type casts to double in AddNumber
	if cJSON_AddStringToObject(header, "ident", <const char *>header_stream.get_chars(8)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "net_prot", header_stream.get_uint32()) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "dem_prot", header_stream.get_uint32()) == NULL: json_err = 1
	if cJSON_AddStringToObject(header, "host_addr", <const char *>header_stream.get_chars(260)) == NULL: json_err = 1
	if cJSON_AddStringToObject(header, "client_id", <const char *>header_stream.get_chars(260)) == NULL: json_err = 1
	if cJSON_AddStringToObject(header, "map_name", <const char *>header_stream.get_chars(260)) == NULL: json_err = 1
	if cJSON_AddStringToObject(header, "game_dir", <const char *>header_stream.get_chars(260)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "play_time", header_stream.get_flt32()) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "tick_count", header_stream.get_uint32()) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "frame_count", header_stream.get_uint32()) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "sigon", header_stream.get_uint32()) == NULL: json_err = 1

	if header_stream.ERRORLEVEL != 0:
		p_state.FAILURE |= 0b1
		p_state.RELAYED_CAW_ERR = header_stream.ERRORLEVEL

	if json_err == 1:
		p_state.FAILURE |= 0b10000
