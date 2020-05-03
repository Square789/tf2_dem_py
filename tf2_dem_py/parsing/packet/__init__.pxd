
from libc.stdio cimport FILE
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON cimport cJSON

cdef extern from "tf2_dem_py/parsing/packet/parse_any.hpp" nogil:
	void parse_any(FILE *stream, ParserState *parser_state, cJSON *root_json)
