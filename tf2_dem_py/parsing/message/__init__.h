#ifndef MESSAGE____INIT____H
#define MESSAGE____INIT____H

#include <stdint.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

typedef struct MsgParserBase
{
	void (*parse)(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
	void (*skip)(CharArrayWrapper *caw, ParserState *parser_state);
} MsgParserBase;

extern MsgParserBase *Print;
extern MsgParserBase *ServerInfo;

#endif
