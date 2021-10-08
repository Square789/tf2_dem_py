#ifndef MESSAGE_PACKETENTITIES__H
#define MESSAGE_PACKETENTITIES__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

void PacketEntities_parse(CharArrayWrapper *caw, ParserState *parser_state);
void PacketEntities_skip(CharArrayWrapper *caw, ParserState *parser_state);

void TempEntities_parse(CharArrayWrapper *caw, ParserState *parser_state);
void TempEntities_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
