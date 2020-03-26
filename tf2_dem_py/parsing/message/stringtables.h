#ifndef MESSAGE_STRINGTABLES__H
#define MESSAGE_STRINGTABLES__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

void p_StringTableCreate(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_StringTableCreate(CharArrayWrapper *caw, ParserState *parser_state);

void p_StringTableUpdate(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_StringTableUpdate(CharArrayWrapper *caw, ParserState *parser_state);

#endif
