from libc.stdint cimport uint32_t

from tf2_dem_py.char_array_wrapper cimport CharArrayWrapper
from tf2_dem_py.cJSON cimport cJSON
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef extern from "__init__.h" nogil:
	cdef struct MsgParserBase:
		void (*parse)(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json)
		void (*skip)(CharArrayWrapper *caw, ParserState *parser_state)

	cdef MsgParserBase *Empty
	cdef MsgParserBase *NetTick
	cdef MsgParserBase *SetConVar
	cdef MsgParserBase *SigOnState
	cdef MsgParserBase *Print
	cdef MsgParserBase *ServerInfo
	cdef MsgParserBase *ClassInfo
	cdef MsgParserBase *StringTableCreate
	cdef MsgParserBase *VoiceInit
	cdef MsgParserBase *ParseSounds
	cdef MsgParserBase *SetView
	cdef MsgParserBase *PacketEntities
	cdef MsgParserBase *GameEventList
