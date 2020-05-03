# distutils: language = c++

from libc.stdio cimport FILE
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON cimport cJSON

cdef extern from "tf2_dem_py/parsing/demo_header.hpp" nogil:
	void parse_demo_header(FILE *stream, ParserState *p_state, cJSON *root_json)
