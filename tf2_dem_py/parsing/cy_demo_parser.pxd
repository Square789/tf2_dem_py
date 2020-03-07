from libc.stdio cimport FILE
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef class CyDemoParser():
	cdef:
		ParserState *state
		FILE *stream
		dict out

	cpdef dict parse(self)
