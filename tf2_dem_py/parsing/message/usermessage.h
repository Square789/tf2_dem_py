#ifndef MESSAGE_USERMESSAGE__H
#define MESSAGE_USERMESSAGE__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

void p_UserMessage(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_UserMessage(CharArrayWrapper *caw, ParserState *parser_state);

#endif