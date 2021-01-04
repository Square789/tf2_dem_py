#ifndef MESSAGE_GAMEEVENTS__H
#define MESSAGE_GAMEEVENTS__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void GameEvent_parse(CharArrayWrapper *caw, ParserState *parser_state);
void GameEvent_skip(CharArrayWrapper *caw, ParserState *parser_state);

void GameEventList_parse(CharArrayWrapper *caw, ParserState *parser_state);
void GameEventList_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
