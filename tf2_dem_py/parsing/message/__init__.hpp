#ifndef MESSAGE___INIT____HPP
#define MESSAGE___INIT____HPP

#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

namespace MessageParsers {

class IMsgParserBase
{
	public:
	virtual void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) = 0;
	virtual void skip(CharArrayWrapper *caw, ParserState *parser_state) = 0;

	IMsgParserBase() {};
};

extern IMsgParserBase *cEmpty;
extern IMsgParserBase *cFile;
extern IMsgParserBase *cNetTick;
extern IMsgParserBase *cStringCommand;
extern IMsgParserBase *cSetConVar;
extern IMsgParserBase *cSigOnState;
extern IMsgParserBase *cPrint;
extern IMsgParserBase *cServerInfo;
extern IMsgParserBase *cClassInfo;
extern IMsgParserBase *cStringTableCreate;
extern IMsgParserBase *cStringTableUpdate;
extern IMsgParserBase *cVoiceInit;
extern IMsgParserBase *cVoiceData;
extern IMsgParserBase *cParseSounds;
extern IMsgParserBase *cSetView;
extern IMsgParserBase *cFixAngle;
extern IMsgParserBase *cBspDecal;
extern IMsgParserBase *cUserMessage;
extern IMsgParserBase *cEntity;
extern IMsgParserBase *cGameEvent;
extern IMsgParserBase *cPacketEntities;
extern IMsgParserBase *cTempEntities;
extern IMsgParserBase *cPreFetch;
extern IMsgParserBase *cGameEventList;
extern IMsgParserBase *cGetCvarValue;

}

#endif
