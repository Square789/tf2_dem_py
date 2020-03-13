from libc.stdint cimport uint8_t
from libc.stdio cimport printf

from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport *
from tf2_dem_py.cJSON.cJSON_wrapper cimport *
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef class Print():
	@staticmethod
	cdef void skip(CharArrayWrapper *caw, ParserState *parser_state) nogil:
		cdef size_t dist = CAW_dist_until_null(caw)
		CAW_skip(caw, dist, 0) # no error checks, occurs in message parsing loop

	@staticmethod
	cdef void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) nogil:
		cdef uint8_t *str_ptr = CAW_get_nulltrm_str(caw)

		if CAW_get_errorlevel(caw) != 0:
			parser_state.FAILURE |= 0b1
			parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(caw)
			return

		if cJSON_AddVolatileStringRefToObject(root_json, "printmsglol", <const char *>str_ptr) == NULL:
			parser_state.FAILURE |= 0b10000
			return

cdef class ServerInfo():
	@staticmethod
	cdef void skip(CharArrayWrapper *caw, ParserState *parser_state) nogil:
		CAW_skip(caw, 36, 2)
		for _ in range(4):
			CAW_skip(caw, CAW_dist_until_null(caw), 0)
		CAW_skip(caw, 0, 1)

	@staticmethod
	cdef void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) nogil:
		cdef cJSON *sinfo_json = cJSON_AddObjectToObject(root_json, "serverinfo")
		cdef uint8_t json_err = 0

		if sinfo_json == NULL:
			parser_state.FAILURE |= 0b10000
			return

		if cJSON_AddNumberToObject(sinfo_json, "version", CAW_get_uint16(caw)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "server_count", CAW_get_uint32(caw)) == NULL: json_err = 1
		if cJSON_AddBoolToObject(sinfo_json, "stv", CAW_get_bit(caw)) == NULL: json_err = 1
		if cJSON_AddBoolToObject(sinfo_json, "dedicated", CAW_get_bit(caw)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "max_crc", CAW_get_uint32(caw)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "max_classes", CAW_get_uint16(caw)) == NULL: json_err = 1
		CAW_skip(caw, 16, 0)
		#if cJSON_AddVolatileStringRefToObject(sinfo_json, "map_hash", <const char *>CAW_get_chars(caw, 16)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "player_count", CAW_get_uint8(caw)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "max_player_count", CAW_get_uint8(caw)) == NULL: json_err = 1
		if cJSON_AddNumberToObject(sinfo_json, "interval_per_tick", CAW_get_flt(caw)) == NULL: json_err = 1
		CAW_skip(caw, 1, 0)
		#if cJSON_AddVolatileStringRefToObject(sinfo_json, "platform", <const char *>CAW_get_uint8(caw)) == NULL: json_err = 1
		if cJSON_AddVolatileStringRefToObject(sinfo_json, "game", <const char *>CAW_get_nulltrm_str(caw)) == NULL: json_err = 1
		if cJSON_AddVolatileStringRefToObject(sinfo_json, "map_name", <const char *>CAW_get_nulltrm_str(caw)) == NULL: json_err = 1
		if cJSON_AddVolatileStringRefToObject(sinfo_json, "skybox", <const char *>CAW_get_nulltrm_str(caw)) == NULL: json_err = 1
		if cJSON_AddVolatileStringRefToObject(sinfo_json, "server_name", <const char *>CAW_get_nulltrm_str(caw)) == NULL: json_err = 1
		if cJSON_AddBoolToObject(sinfo_json, "replay", CAW_get_bit(caw)) == NULL: json_err = 1

		if CAW_get_errorlevel(caw) != 0:
			parser_state.FAILURE |= 0b1
			parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(caw)
			return

		if json_err == 1:
			parser_state.FAILURE |= 0b10000
			return
