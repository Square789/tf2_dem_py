#ifndef MESSAGE___INIT____H
#define MESSAGE___INIT____H

#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

typedef struct MsgParserBase
{
	void (*parse)(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
	void (*skip)(CharArrayWrapper *caw, ParserState *parser_state);
} MsgParserBase;

extern MsgParserBase *Empty;
extern MsgParserBase *File;
extern MsgParserBase *NetTick;
extern MsgParserBase *SetConVar;
extern MsgParserBase *SigOnState;
extern MsgParserBase *Print;
extern MsgParserBase *ServerInfo;
extern MsgParserBase *ClassInfo;
extern MsgParserBase *StringTableCreate;
extern MsgParserBase *StringTableUpdate;
extern MsgParserBase *VoiceInit;
extern MsgParserBase *ParseSounds;
extern MsgParserBase *SetView;
extern MsgParserBase *UserMessage;
extern MsgParserBase *Entity;
extern MsgParserBase *GameEvent;
extern MsgParserBase *PacketEntities;
extern MsgParserBase *TempEntities;
extern MsgParserBase *PreFetch;
extern MsgParserBase *GameEventList;

#endif
