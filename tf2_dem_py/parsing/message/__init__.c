
#include "tf2_dem_py/parsing/message/__init__.h"
#include "tf2_dem_py/parsing/message/various.h"

MsgParserBase Print_s = {p_Print, s_Print};
MsgParserBase *Print = &Print_s;

MsgParserBase ServerInfo_s = {p_ServerInfo, s_ServerInfo};
MsgParserBase *ServerInfo = &ServerInfo_s;
