
from libc.stdint cimport uint8_t, uint16_t

from tf2_dem_py.char_array_wrapper cimport CharArrayWrapper
from tf2_dem_py.cJSON cimport cJSON
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef extern from "tf2_dem_py/parsing/usermessage/saytext2.h":
	void process_SayText2(CharArrayWrapper *caw, ParserState *p_state, cJSON *tgt_json)
