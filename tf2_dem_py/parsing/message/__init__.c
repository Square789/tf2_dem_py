
#include "tf2_dem_py/parsing/message/__init__.h"

#include "tf2_dem_py/parsing/message/various.h"
#include "tf2_dem_py/parsing/message/stringtables.h"

MsgParserBase Empty_s = {p_Empty, s_Empty};
MsgParserBase *Empty = &Empty_s;

MsgParserBase NetTick_s = {p_NetTick, s_NetTick};
MsgParserBase *NetTick = &NetTick_s;

MsgParserBase Print_s = {p_Print, s_Print};
MsgParserBase *Print = &Print_s;

MsgParserBase ServerInfo_s = {p_ServerInfo, s_ServerInfo};
MsgParserBase *ServerInfo = &ServerInfo_s;

MsgParserBase StringTableCreate_s = {p_StringTableCreate, s_StringTableCreate};
MsgParserBase *StringTableCreate = &StringTableCreate_s;
