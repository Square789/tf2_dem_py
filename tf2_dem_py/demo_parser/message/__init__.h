#ifndef MESSAGE___INIT____H
#define MESSAGE___INIT____H

#include <stdint.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

typedef struct MsgParser_s {
	void (*parse)(CharArrayWrapper *caw, ParserState *parser_state);
	void (*skip)(CharArrayWrapper *caw, ParserState *parser_state);
} MsgParser;

extern const MsgParser MsgParser_Empty;
extern const MsgParser MsgParser_File;
extern const MsgParser MsgParser_NetTick;
extern const MsgParser MsgParser_StringCommand;
extern const MsgParser MsgParser_SetConVar;
extern const MsgParser MsgParser_SigOnState;
extern const MsgParser MsgParser_Print;
extern const MsgParser MsgParser_ServerInfo;
extern const MsgParser MsgParser_ClassInfo;
extern const MsgParser MsgParser_StringTableCreate;
extern const MsgParser MsgParser_StringTableUpdate;
extern const MsgParser MsgParser_VoiceInit;
extern const MsgParser MsgParser_VoiceData;
extern const MsgParser MsgParser_ParseSounds;
extern const MsgParser MsgParser_SetView;
extern const MsgParser MsgParser_FixAngle;
extern const MsgParser MsgParser_BspDecal;
extern const MsgParser MsgParser_UserMessage;
extern const MsgParser MsgParser_Entity;
extern const MsgParser MsgParser_GameEvent;
extern const MsgParser MsgParser_PacketEntities;
extern const MsgParser MsgParser_TempEntities;
extern const MsgParser MsgParser_PreFetch;
extern const MsgParser MsgParser_GameEventList;
extern const MsgParser MsgParser_GetCvarValue;

#endif
