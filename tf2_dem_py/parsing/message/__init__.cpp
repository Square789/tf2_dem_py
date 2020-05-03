
#include "tf2_dem_py/parsing/message/__init__.hpp"

#include "tf2_dem_py/parsing/message/classinfo.hpp"
#include "tf2_dem_py/parsing/message/gameevents.hpp"
#include "tf2_dem_py/parsing/message/packetentities.hpp"
#include "tf2_dem_py/parsing/message/stringtables.hpp"
#include "tf2_dem_py/parsing/message/usermessage.hpp"
#include "tf2_dem_py/parsing/message/various.hpp"
#include "tf2_dem_py/parsing/message/voice.hpp"

namespace MessageParsers {

IMsgParserBase *cEmpty             = new MessageParsers::Empty(); // 0 //
IMsgParserBase *cFile              = new MessageParsers::File(); // 2 //
IMsgParserBase *cNetTick           = new MessageParsers::NetTick(); // 3 //
IMsgParserBase *cStringCommand     = new MessageParsers::StringCommand(); // 4 //
IMsgParserBase *cSetConVar         = new MessageParsers::SetConVar(); // 5 //
IMsgParserBase *cSigOnState        = new MessageParsers::SigOnState(); // 6 //
IMsgParserBase *cPrint             = new MessageParsers::Print(); // 7 //
IMsgParserBase *cServerInfo        = new MessageParsers::ServerInfo(); // 8 //
IMsgParserBase *cClassInfo         = new MessageParsers::ClassInfo(); // 10 //
IMsgParserBase *cStringTableCreate = new MessageParsers::StringTableCreate(); // 12 //
IMsgParserBase *cStringTableUpdate = new MessageParsers::StringTableUpdate(); // 13 //
IMsgParserBase *cVoiceInit         = new MessageParsers::VoiceInit(); // 14 //
IMsgParserBase *cVoiceData         = new MessageParsers::VoiceData(); // 15 //
IMsgParserBase *cParseSounds       = new MessageParsers::ParseSounds(); // 17 //
IMsgParserBase *cSetView           = new MessageParsers::SetView(); // 18 //
IMsgParserBase *cFixAngle          = new MessageParsers::FixAngle(); // 19 //
IMsgParserBase *cBspDecal          = new MessageParsers::BspDecal(); // 21 //
IMsgParserBase *cUserMessage       = new MessageParsers::UserMessage(); // 23 //
IMsgParserBase *cEntity            = new MessageParsers::Entity(); // 24 //
IMsgParserBase *cGameEvent         = new MessageParsers::GameEvent(); // 25 //
IMsgParserBase *cPacketEntities    = new MessageParsers::PacketEntities(); // 26 //
IMsgParserBase *cTempEntities      = new MessageParsers::TempEntities(); // 27 //
IMsgParserBase *cPreFetch          = new MessageParsers::PreFetch(); // 28 //
IMsgParserBase *cGameEventList     = new MessageParsers::GameEventList(); // 30 //
IMsgParserBase *cGetCvarValue      = new MessageParsers::GetCvarValue(); // 31 //

}
