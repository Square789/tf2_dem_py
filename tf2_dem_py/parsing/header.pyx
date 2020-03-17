from libc.stdio cimport FILE

from tf2_dem_py.char_array_wrapper cimport *
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON cimport (cJSON, cJSON_CreateObject,
	cJSON_AddNumberToObject, cJSON_AddVolatileStringRefToObject, cJSON_AddObjectToObject)

cdef void parse(FILE *stream, ParserState* p_state, cJSON *root_json):
	"""
	Parses the demo header from an open file stream.
	"""
	cdef char json_err = 0

	cdef CharArrayWrapper *header_caw = CAW_create_new(stream, 1072)
	cdef cJSON *header = cJSON_AddObjectToObject(root_json, "header")

	# Implicit type casts to double in AddNumber
	if cJSON_AddVolatileStringRefToObject(header, "ident", <const char *>CAW_get_chars(header_caw, 8)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "net_prot", CAW_get_uint32(header_caw)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "dem_prot", CAW_get_uint32(header_caw)) == NULL: json_err = 1
	if cJSON_AddVolatileStringRefToObject(header, "host_addr", <const char *>CAW_get_chars(header_caw, 260)) == NULL: json_err = 1
	if cJSON_AddVolatileStringRefToObject(header, "client_id", <const char *>CAW_get_chars(header_caw, 260)) == NULL: json_err = 1
	if cJSON_AddVolatileStringRefToObject(header, "map_name", <const char *>CAW_get_chars(header_caw, 260)) == NULL: json_err = 1
	if cJSON_AddVolatileStringRefToObject(header, "game_dir", <const char *>CAW_get_chars(header_caw, 260)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "play_time", CAW_get_flt(header_caw)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "tick_count", CAW_get_uint32(header_caw)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "frame_count", CAW_get_uint32(header_caw)) == NULL: json_err = 1
	if cJSON_AddNumberToObject(header, "sigon", CAW_get_uint32(header_caw)) == NULL: json_err = 1

	if CAW_get_errorlevel(header_caw) != 0:
		p_state.FAILURE |= 0b1
		p_state.RELAYED_CAW_ERR = CAW_get_errorlevel(header_caw)

	if json_err == 1:
		p_state.FAILURE |= 0b10000
