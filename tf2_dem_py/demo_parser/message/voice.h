#ifndef MESSAGE_VOICE__H
#define MESSAGE_VOICE__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

void ParseSounds_parse(CharArrayWrapper *caw, ParserState *parser_state);
void ParseSounds_skip(CharArrayWrapper *caw, ParserState *parser_state);

void VoiceInit_parse(CharArrayWrapper *caw, ParserState *parser_state);
void VoiceInit_skip(CharArrayWrapper *caw, ParserState *parser_state);

void VoiceData_parse(CharArrayWrapper *caw, ParserState *parser_state);
void VoiceData_skip(CharArrayWrapper *caw, ParserState *parser_state);

#endif
