# distutils: language = c++

from libc.stdint cimport uint32_t

from tf2_dem_py.char_array_wrapper cimport CharArrayWrapper
from tf2_dem_py.cJSON cimport cJSON
from tf2_dem_py.parsing.parser_state cimport ParserState

cdef extern from "tf2_dem_py/parsing/message/__init__.hpp" namespace "MessageParsers" nogil:
	cdef cppclass IMsgParserBase:
		void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json)
		void skip(CharArrayWrapper *caw, ParserState *parser_state)

	cdef IMsgParserBase *cEmpty
	cdef IMsgParserBase *cFile
	cdef IMsgParserBase *cNetTick
	cdef IMsgParserBase *cStringCommand
	cdef IMsgParserBase *cSetConVar
	cdef IMsgParserBase *cSigOnState
	cdef IMsgParserBase *cPrint
	cdef IMsgParserBase *cServerInfo
	cdef IMsgParserBase *cClassInfo
	cdef IMsgParserBase *cStringTableCreate
	cdef IMsgParserBase *cStringTableUpdate
	cdef IMsgParserBase *cVoiceInit
	cdef IMsgParserBase *cVoiceData
	cdef IMsgParserBase *cParseSounds
	cdef IMsgParserBase *cSetView
	cdef IMsgParserBase *cFixAngle
	cdef IMsgParserBase *cBspDecal
	cdef IMsgParserBase *cUserMessage
	cdef IMsgParserBase *cEntity
	cdef IMsgParserBase *cGameEvent
	cdef IMsgParserBase *cPacketEntities
	cdef IMsgParserBase *cTempEntities
	cdef IMsgParserBase *cPreFetch
	cdef IMsgParserBase *cGameEventList
	cdef IMsgParserBase *cGetCvarValue
