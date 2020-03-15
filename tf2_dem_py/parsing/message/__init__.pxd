from libc.stdint cimport uint32_t

from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport CharArrayWrapper
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef extern from "__init__.h" nogil:
	cdef struct MsgParserBase:
		void (*parse)(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json)
		void (*skip)(CharArrayWrapper *caw, ParserState *parser_state)

	cdef MsgParserBase *Empty
	cdef MsgParserBase *NetTick
	cdef MsgParserBase *Print
	cdef MsgParserBase *ServerInfo
	cdef MsgParserBase *StringTableCreate
