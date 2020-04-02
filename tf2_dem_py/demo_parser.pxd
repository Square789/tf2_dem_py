from libc.stdio cimport FILE
from tf2_dem_py.parsing.parser_state cimport ParserState

from tf2_dem_py.cJSON cimport cJSON

cdef class DemoParser():
	cdef:
		ParserState *state
		FILE *stream
		cJSON *json_obj

	cpdef dict parse(self)
	cdef void cleanup(self)
