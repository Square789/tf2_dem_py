
#include "tf2_dem_py/parsing/message/__init__.h"

#include "tf2_dem_py/parsing/message/classinfo.h"
#include "tf2_dem_py/parsing/message/gameevents.h"
#include "tf2_dem_py/parsing/message/packetentities.h"
#include "tf2_dem_py/parsing/message/stringtables.h"
#include "tf2_dem_py/parsing/message/various.h"
#include "tf2_dem_py/parsing/message/voice.h"

MsgParserBase Empty_s = {p_Empty, s_Empty};
MsgParserBase *Empty = &Empty_s; // 0 //

MsgParserBase NetTick_s = {p_NetTick, s_NetTick};
MsgParserBase *NetTick = &NetTick_s; // 3 //

MsgParserBase SetConVar_s = {p_SetConVar, s_SetConVar};
MsgParserBase *SetConVar = &SetConVar_s; // 5 //

MsgParserBase SigOnState_s = {p_SigOnState, s_SigOnState};
MsgParserBase *SigOnState = &SigOnState_s; // 6 //

MsgParserBase Print_s = {p_Print, s_Print};
MsgParserBase *Print = &Print_s; // 7 //

MsgParserBase ServerInfo_s = {p_ServerInfo, s_ServerInfo};
MsgParserBase *ServerInfo = &ServerInfo_s; // 8 //

MsgParserBase ClassInfo_s = {p_ClassInfo, s_ClassInfo};
MsgParserBase *ClassInfo = &ClassInfo_s; // 10 //

MsgParserBase StringTableCreate_s = {p_StringTableCreate, s_StringTableCreate};
MsgParserBase *StringTableCreate = &StringTableCreate_s; // 12 //

MsgParserBase VoiceInit_s = {p_VoiceInit, s_VoiceInit};
MsgParserBase *VoiceInit = &VoiceInit_s; // 14 //

MsgParserBase ParseSounds_s = {p_ParseSounds, s_ParseSounds};
MsgParserBase *ParseSounds = &ParseSounds_s; // 17 //

MsgParserBase SetView_s = {p_SetView, s_SetView};
MsgParserBase *SetView = &SetView_s; // 18 //

MsgParserBase PacketEntities_s = {p_PacketEntities, s_PacketEntities};
MsgParserBase *PacketEntities = &PacketEntities_s; // 26 //

MsgParserBase GameEventList_s = {p_GameEventList, s_GameEventList};
MsgParserBase *GameEventList = &GameEventList_s; // 30 //
