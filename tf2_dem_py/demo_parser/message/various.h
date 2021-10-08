#ifndef MESSAGE_VARIOUS__H
#define MESSAGE_VARIOUS__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

void Empty_parse(CharArrayWrapper *caw, ParserState *parser_state);
void Empty_skip(CharArrayWrapper *caw, ParserState *parser_state);

void File_parse(CharArrayWrapper *caw, ParserState *parser_state);
void File_skip(CharArrayWrapper *caw, ParserState *parser_state);

void NetTick_parse(CharArrayWrapper *caw, ParserState *parser_state);
void NetTick_skip(CharArrayWrapper *caw, ParserState *parser_state);

void StringCommand_parse(CharArrayWrapper *caw, ParserState *parser_state);
void StringCommand_skip(CharArrayWrapper *caw, ParserState *parser_state);

void SetConVar_parse(CharArrayWrapper *caw, ParserState *parser_state);
void SetConVar_skip(CharArrayWrapper *caw, ParserState *parser_state);

void SigOnState_parse(CharArrayWrapper *caw, ParserState *parser_state);
void SigOnState_skip(CharArrayWrapper *caw, ParserState *parser_state);

void Print_parse(CharArrayWrapper *caw, ParserState *parser_state);
void Print_skip(CharArrayWrapper *caw, ParserState *parser_state);

void ServerInfo_parse(CharArrayWrapper *caw, ParserState *parser_state);
void ServerInfo_skip(CharArrayWrapper *caw, ParserState *parser_state);

void SetView_parse(CharArrayWrapper *caw, ParserState *parser_state);
void SetView_skip(CharArrayWrapper *caw, ParserState *parser_state);

void FixAngle_parse(CharArrayWrapper *caw, ParserState *parser_state);
void FixAngle_skip(CharArrayWrapper *caw, ParserState *parser_state);

void BspDecal_parse(CharArrayWrapper *caw, ParserState *parser_state);
void BspDecal_skip(CharArrayWrapper *caw, ParserState *parser_state);

void Entity_parse(CharArrayWrapper *caw, ParserState *parser_state);
void Entity_skip(CharArrayWrapper *caw, ParserState *parser_state);

void PreFetch_parse(CharArrayWrapper *caw, ParserState *parser_state);
void PreFetch_skip(CharArrayWrapper *caw, ParserState *parser_state);

void GetCvarValue_parse(CharArrayWrapper *caw, ParserState *parser_state);
void GetCvarValue_skip(CharArrayWrapper *caw, ParserState *parser_state);


#endif
