#ifndef MESSAGE_VARIOUS__H
#define MESSAGE_VARIOUS__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

void p_Print(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_Print(CharArrayWrapper *caw, ParserState *parser_state);

void p_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state);

#endif
