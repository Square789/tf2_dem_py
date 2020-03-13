from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport CharArrayWrapper
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef class Print():
	@staticmethod
	cdef void skip(CharArrayWrapper *caw, ParserState *parser_state) nogil

	@staticmethod
	cdef void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) nogil

cdef class ServerInfo():
	@staticmethod
	cdef void skip(CharArrayWrapper *caw, ParserState *parser_state) nogil

	@staticmethod
	cdef void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) nogil
