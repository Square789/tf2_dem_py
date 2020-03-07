from libc.stdio cimport FILE

from tf2_dem_py.parsing.parser_state cimport ParserState

cdef dict parse(FILE *stream, ParserState *p_state)
