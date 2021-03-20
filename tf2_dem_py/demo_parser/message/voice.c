#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/message/voice.h"

void ParseSounds_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	ParseSounds_skip(caw, parser_state);
}

void ParseSounds_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t reliable = CharArrayWrapper_get_bit(caw);
	// uint8_t num;
	uint16_t length = 0;
	if (reliable == 1) {
		// num = 1;
		length = (uint16_t)CharArrayWrapper_get_uint8(caw);
	} else {
		// num = CharArrayWrapper_get_uint8(caw);
		CharArrayWrapper_skip(caw, 1, 0);
		length = CharArrayWrapper_get_uint16(caw);
	}
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}


void VoiceInit_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	VoiceInit_skip(caw, parser_state);
}

void VoiceInit_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	if (CharArrayWrapper_get_uint8(caw) == 0xFF) {
		CharArrayWrapper_skip(caw, 2, 0);
	}
}


void VoiceData_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	VoiceData_skip(caw, parser_state);
}

void VoiceData_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len;
	CharArrayWrapper_skip(caw, 2, 0);
	len = CharArrayWrapper_get_uint16(caw);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}
