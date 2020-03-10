from libc.stdio cimport FILE
from tf2_dem_py.parsing.parser_state cimport ParserState

from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef class CyDemoParser():
	cdef:
		ParserState *state
		FILE *stream
		cJSON *json_obj

	cpdef dict parse(self)
