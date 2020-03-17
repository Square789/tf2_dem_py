from libc.stdio cimport FILE

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON cimport cJSON

cdef void parse(FILE *stream, ParserState *p_state, cJSON *root_json)
