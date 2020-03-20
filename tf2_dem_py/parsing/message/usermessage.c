
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state.h"

#include "tf2_dem_py/parsing/message/usermessage.h"

void p_UserMessage(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint8_t user_message_type = CAW_get_uint8(caw);
	uint16_t len;
	CAW_read_raw(caw, &len, 1, 3);

	uint8_t c;
	uint8_t r;
	uint8_t *chat = NULL;
	uint8_t *from = NULL;
	uint8_t *mesg = NULL; //lol
	size_t i = 0;

	size_t buflen;
	if (len % 8 == 0) {
		buflen = (len / 8);
	} else {
		buflen = (len / 8) + 1;
	}

	uint8_t *user_msg_buf = (uint8_t *)malloc(buflen);
	if (user_msg_buf == NULL) {
		parser_state->FAILURE |= 0b1000000;
		return;
	}
	CAW_read_raw(caw, user_msg_buf, len / 8, len % 8);
	if (caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= 0b1;
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		return;
	}
	CharArrayWrapper *user_message_caw = CAW_from_buffer((void *)user_msg_buf, buflen);
	if (user_message_caw == NULL) {
		parser_state->FAILURE |= 0b1000000;
		return;
	}

	switch (user_message_type) {
	case 4:
		c = CAW_get_uint8(user_message_caw);
		r = CAW_get_uint8(user_message_caw);
		chat = CAW_get_chars(user_message_caw, CAW_dist_until_null(user_message_caw));
		from = CAW_get_chars(user_message_caw, CAW_dist_until_null(user_message_caw));
		mesg = CAW_get_chars(user_message_caw, CAW_dist_until_null(user_message_caw));
		printf("%u %u] in <", c, r, chat);
		i = 0;
		while (1) {
			printf("%x ", chat[i]);
			if (chat[i] == 0) { break; }
			i++;
		}
		printf(">: ");
		i = 0;
		while (1) {
			printf("%x ", from[i]);
			if (from[i] == 0) { break; }
			i++;
		}
		printf(": ");
		i = 0;
		while (1) {
			printf("%x ", mesg[i]);
			if (mesg[i] == 0) { break; }
			i++;
		}
		printf("\n");
		break;
	default:
		break;
	if (user_message_caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= 0b1;
		parser_state->RELAYED_CAW_ERR = user_message_caw->ERRORLEVEL;
		// This may be ambigous in really odd cases where there is a message
		// length discrepancy and the inner caw sets the  out of bounds flag.
	}
	CAW_delete(user_message_caw);
	}
}

void s_UserMessage(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 0);
	uint16_t len;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}
