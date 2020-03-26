#ifndef MESSAGE_VOICE__H
#define MESSAGE_VOICE__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

void p_VoiceInit(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_VoiceInit(CharArrayWrapper *caw, ParserState *parser_state);

void p_VoiceData(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_VoiceData(CharArrayWrapper *caw, ParserState *parser_state);

void p_ParseSounds(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_ParseSounds(CharArrayWrapper *caw, ParserState *parser_state);

#endif
