#ifndef MESSAGE_CLASSINFO__H
#define MESSAGE_CLASSINFO__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

void ClassInfo_parse(CharArrayWrapper *caw, ParserState *parser_state);
void ClassInfo_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
