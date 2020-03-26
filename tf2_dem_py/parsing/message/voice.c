
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/voice.h"

void p_ParseSounds(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_ParseSounds(caw, parser_state);
}

void s_ParseSounds(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t reliable = CAW_get_bit(caw);
	uint8_t num;
	uint16_t length;
	if (reliable == 1) {
		num = 1;
		length = (uint16_t)CAW_get_uint8(caw);
	} else {
		num = CAW_get_uint8(caw);
		length = CAW_get_uint16(caw);
	}
	CAW_skip(caw, length / 8, length % 8);
}


void p_VoiceInit(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_VoiceInit(caw, parser_state);
}

void s_VoiceInit(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, CAW_dist_until_null(caw), 0);
	if (CAW_get_uint8(caw) == 255) {
		CAW_skip(caw, 2, 0);
	}
}


void p_VoiceData(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_VoiceData(caw, parser_state);
}

void s_VoiceData(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CAW_skip(caw, 2, 0);
	CAW_read_raw(caw, &len, 2, 0);
	CAW_skip(caw, len / 8, len % 8);
}
