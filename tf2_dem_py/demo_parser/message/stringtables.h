#ifndef MESSAGE_STRINGTABLES__H
#define MESSAGE_STRINGTABLES__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void StringTableCreate_parse(CharArrayWrapper *caw, ParserState *parser_state);
void StringTableCreate_skip(CharArrayWrapper *caw, ParserState *parser_state);

void StringTableUpdate_parse(CharArrayWrapper *caw, ParserState *parser_state);
void StringTableUpdate_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
