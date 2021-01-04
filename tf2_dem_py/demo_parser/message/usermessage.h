#ifndef MESSAGE_USERMESSAGE__H
#define MESSAGE_USERMESSAGE__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void UserMessage_parse(CharArrayWrapper *caw, ParserState *parser_state);
void UserMessage_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
