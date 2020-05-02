#ifndef MESSAGE___INIT____H
#define MESSAGE___INIT____H

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

	private:
	IMsgParserBase() {};
};

}
// extern MsgParserBase *Empty;
// extern MsgParserBase *File;
// extern MsgParserBase *NetTick;
// extern MsgParserBase *StringCommand;
// extern MsgParserBase *SetConVar;
// extern MsgParserBase *SigOnState;
// extern MsgParserBase *Print;
// extern MsgParserBase *ServerInfo;
// extern MsgParserBase *ClassInfo;
// extern MsgParserBase *StringTableCreate;
// extern MsgParserBase *StringTableUpdate;
// extern MsgParserBase *VoiceInit;
// extern MsgParserBase *VoiceData;
// extern MsgParserBase *ParseSounds;
// extern MsgParserBase *SetView;
// extern MsgParserBase *FixAngle;
// extern MsgParserBase *BspDecal;
// extern MsgParserBase *UserMessage;
// extern MsgParserBase *Entity;
// extern MsgParserBase *GameEvent;
// extern MsgParserBase *PacketEntities;
// extern MsgParserBase *TempEntities;
// extern MsgParserBase *PreFetch;
// extern MsgParserBase *GameEventList;
// extern MsgParserBase *GetCvarValue;

#endif
