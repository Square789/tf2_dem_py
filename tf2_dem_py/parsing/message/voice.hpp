#ifndef MESSAGE_VOICE__HPP
#define MESSAGE_VOICE__HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/message/__init__.hpp"

using ParserState::ParserState_c;

namespace MessageParsers {

class ParseSounds: public IMsgParserBase {
	void parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict);
	void skip(CharArrayWrapper *caw, ParserState_c *parser_state);
};

class VoiceInit: public IMsgParserBase {
	void parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict);
	void skip(CharArrayWrapper *caw, ParserState_c *parser_state);
};

class VoiceData: public IMsgParserBase {
	void parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict);
	void skip(CharArrayWrapper *caw, ParserState_c *parser_state);
};

}

#endif
