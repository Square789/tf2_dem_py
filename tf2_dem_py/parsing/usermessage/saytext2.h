#ifndef USERMESSAGE_SAYTEXT2__H
#define USERMESSAGE_SAYTEXT2__H

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

void process_SayText2(CharArrayWrapper *caw, ParserState *p_state, cJSON *tgt_json);

#endif
