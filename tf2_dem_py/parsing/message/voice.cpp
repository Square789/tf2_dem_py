
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/message/voice.hpp"

using ParserState::ParserState_c;

namespace MessageParsers {

void ParseSounds::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void ParseSounds::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	uint8_t reliable = caw->get_bit();
	uint8_t num;
	uint16_t length;
	if (reliable == 1) {
		num = 1;
		length = (uint16_t)caw->get_uint8();
	} else {
		num = caw->get_uint8();
		length = caw->get_uint16();
	}
	caw->skip(length / 8, length % 8);
}


void VoiceInit::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void VoiceInit::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	caw->skip(caw->dist_until_null(), 0);
	if (caw->get_uint8() == 255) {
		caw->skip(2, 0);
	}
}


void VoiceData::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void VoiceData::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	uint16_t len = 0;
	caw->skip(2, 0);
	caw->read_raw(&len, 2, 0);
	caw->skip(len / 8, len % 8);
	}

}
