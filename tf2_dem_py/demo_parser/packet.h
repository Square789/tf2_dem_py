#ifndef PACKET__H
#define PACKET__H

#include <stdio.h>
#include "tf2_dem_py/demo_parser/parser_state.h"

typedef struct ParserState_s ParserState; // ugly fwd declaration 

void Consolecmd_parse(FILE *stream, ParserState *parser_state);
void Datatables_parse(FILE *stream, ParserState *parser_state);
void Message_parse(FILE *stream, ParserState *parser_state);
void Synctick_parse(FILE *stream, ParserState *parser_state);
void Usercmd_parse(FILE *stream, ParserState *parser_state);
void Stringtables_parse(FILE *stream, ParserState *parser_state);

#endif
