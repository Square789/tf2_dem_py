from libc.stdio cimport FILE, fread, fseek, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState

cdef void parse(FILE *stream, ParserState *p_state):
	cdef uint32_t tick

	# That is all
	fread(&tick, sizeof(tick), 1, stream)
