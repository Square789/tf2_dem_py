from libc.stdio cimport FILE

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef void parse(FILE *stream, ParserState *parser_state, cJSON *root_json)
