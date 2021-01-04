
#include "tf2_dem_py/demo_parser/message/__init__.h"

#include "tf2_dem_py/demo_parser/message/classinfo.h"
#include "tf2_dem_py/demo_parser/message/gameevents.h"
#include "tf2_dem_py/demo_parser/message/packetentities.h"
#include "tf2_dem_py/demo_parser/message/stringtables.h"
#include "tf2_dem_py/demo_parser/message/usermessage.h"
#include "tf2_dem_py/demo_parser/message/various.h"
#include "tf2_dem_py/demo_parser/message/voice.h"

const MsgParser MsgParser_Empty             = {&Empty_parse,             &Empty_skip};             //  0
const MsgParser MsgParser_File              = {&File_parse,              &File_skip};              //  2
const MsgParser MsgParser_NetTick           = {&NetTick_parse,           &NetTick_skip};           //  3
const MsgParser MsgParser_StringCommand     = {&StringCommand_parse,     &StringCommand_skip};     //  4
const MsgParser MsgParser_SetConVar         = {&SetConVar_parse,         &SetConVar_skip};         //  5
const MsgParser MsgParser_SigOnState        = {&SigOnState_parse,        &SigOnState_skip};        //  6
const MsgParser MsgParser_Print             = {&Print_parse,             &Print_skip};             //  7
const MsgParser MsgParser_ServerInfo        = {&ServerInfo_parse,        &ServerInfo_skip};        //  8
const MsgParser MsgParser_ClassInfo         = {&ClassInfo_parse,         &ClassInfo_skip};         // 10
const MsgParser MsgParser_StringTableCreate = {&StringTableCreate_parse, &StringTableCreate_skip}; // 12
const MsgParser MsgParser_StringTableUpdate = {&StringTableUpdate_parse, &StringTableUpdate_skip}; // 13
const MsgParser MsgParser_VoiceInit         = {&VoiceInit_parse,         &VoiceInit_skip};         // 14
const MsgParser MsgParser_VoiceData         = {&VoiceData_parse,         &VoiceData_skip};         // 15
const MsgParser MsgParser_ParseSounds       = {&ParseSounds_parse,       &ParseSounds_skip};       // 17
const MsgParser MsgParser_SetView           = {&SetView_parse,           &SetView_skip};           // 18
const MsgParser MsgParser_FixAngle          = {&FixAngle_parse,          &FixAngle_skip};          // 19
const MsgParser MsgParser_BspDecal          = {&BspDecal_parse,          &BspDecal_skip};          // 21
const MsgParser MsgParser_UserMessage       = {&UserMessage_parse,       &UserMessage_skip};       // 23
const MsgParser MsgParser_Entity            = {&Entity_parse,            &Entity_skip};            // 24
const MsgParser MsgParser_GameEvent         = {&GameEvent_parse,         &GameEvent_skip};         // 25
const MsgParser MsgParser_PacketEntities    = {&PacketEntities_parse,    &PacketEntities_skip};    // 26
const MsgParser MsgParser_TempEntities      = {&TempEntities_parse,      &TempEntities_skip};      // 27
const MsgParser MsgParser_PreFetch          = {&PreFetch_parse,          &PreFetch_skip};          // 28
const MsgParser MsgParser_GameEventList     = {&GameEventList_parse,     &GameEventList_skip};     // 30
const MsgParser MsgParser_GetCvarValue      = {&GetCvarValue_parse,      &GetCvarValue_skip};      // 31
