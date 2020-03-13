from libc.stdint cimport uint8_t

from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport *
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON, cJSON_AddVolatileStringRefToObject
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef class Print():
	@staticmethod
	cdef void skip(CharArrayWrapper *caw, ParserState *parser_state) nogil:
		cdef size_t dist = CAW_dist_until_null(caw)

		if CAW_get_errorlevel(caw) != 0:
			parser_state.FAILURE |= 0b1
			parser_state.RELAYED_CAW_ERR = CAW_get_errorlevel(caw)
			return

		CAW_skip(caw, dist, 0) # no error checks, should be good as dist_until_null returns skippable length

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
