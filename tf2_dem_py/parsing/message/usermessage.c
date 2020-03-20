
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/usermessage.h"

void p_UserMessage(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint8_t user_message_type = CAW_get_uint8(caw);
	uint16_t len;

	uint8_t c;
	uint8_t r;
	uint8_t *chat;
	uint8_t *from;
	uint8_t *mesg; //lol

	CAW_read_raw(caw, &len, 1, 3);
	uint8_t *user_msg_buf = CAW_get_chars(caw, );

	switch (user_message_type) {
	case 4:
		c = CAW_get_uint8(caw);
		r = CAW_get_uint8(caw);
		chat = CAW_get_chars(caw, CAW_dist_until_null(caw));
		from = CAW_get_chars(caw, CAW_dist_until_null(caw));
		mesg = CAW_get_chars(caw, CAW_dist_until_null(caw));
		printf("%u %u] %s : %s : %s", c, r, chat, from, mesg);
		break;
	default:
		CAW_skip(caw, len / 8, len % 8);
		break;
	}
}

void s_UserMessage(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 0);
	uint16_t len;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}
