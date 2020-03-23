#ifndef MESSAGE_VARIOUS__H
#define MESSAGE_VARIOUS__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

void p_Empty(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_Empty(CharArrayWrapper *caw, ParserState *parser_state);

void p_File(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_File(CharArrayWrapper *caw, ParserState *parser_state);

void p_NetTick(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_NetTick(CharArrayWrapper *caw, ParserState *parser_state);

void p_StringCommand(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_StringCommand(CharArrayWrapper *caw, ParserState *parser_state);

void p_SetConVar(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_SetConVar(CharArrayWrapper *caw, ParserState *parser_state);

void p_SigOnState(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_SigOnState(CharArrayWrapper *caw, ParserState *parser_state);

void p_Print(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_Print(CharArrayWrapper *caw, ParserState *parser_state);

void p_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_ServerInfo(CharArrayWrapper *caw, ParserState *parser_state);

void p_SetView(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_SetView(CharArrayWrapper *caw, ParserState *parser_state);

void p_FixAngle(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_FixAngle(CharArrayWrapper *caw, ParserState *parser_state);

void p_BspDecal(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_BspDecal(CharArrayWrapper *caw, ParserState *parser_state);

void p_Entity(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_Entity(CharArrayWrapper *caw, ParserState *parser_state);

void p_PreFetch(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_PreFetch(CharArrayWrapper *caw, ParserState *parser_state);

#endif
