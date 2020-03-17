from libc.stdio cimport FILE, fread, fseek, ferror, feof, SEEK_CUR
from libc.stdint cimport uint8_t, uint32_t

from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.cJSON cimport cJSON

cdef void parse(FILE *stream, ParserState *p_state, cJSON *root_json):
	cdef uint32_t tick

	# That is all
	fread(&tick, sizeof(tick), 1, stream)
