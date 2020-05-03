#ifndef MESSAGE_USERMESSAGE__HPP
#define MESSAGE_USERMESSAGE__HPP

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/__init__.hpp"

namespace MessageParsers {

class UserMessage: public IMsgParserBase {
	void parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
	void skip(CharArrayWrapper *caw, ParserState *parser_state);
};

}

#endif