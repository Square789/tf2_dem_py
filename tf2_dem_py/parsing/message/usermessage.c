
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
		if (parser_state->flags & 0b1) {
			handle_say_text(user_message_caw, parser_state, cJSON_GetObjectItemCaseSensitive(root_json, "chat"));
			// ...
		}
		break;
	default:
		break;
	}
	if (user_message_caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= 0b1;
		parser_state->RELAYED_CAW_ERR = user_message_caw->ERRORLEVEL;
		// This may be ambigous in really odd cases where there is a message
		// length discrepancy and the inner caw sets the out of bounds flag.
	}
	CAW_delete(user_message_caw);
}

void s_UserMessage(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 0);
	uint16_t len;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}

inline void handle_say_text(CharArrayWrapper *um_caw, ParserState *parser_state, cJSON *json_chat_array) {
	uint8_t client;
	uint8_t r;
	uint8_t is_senderless;
	uint8_t *chat = NULL;
	uint8_t *from = NULL;
	uint8_t *mesg = NULL;
	cJSON *message_json = NULL;
	uint8_t json_err = 0;

	client = CAW_get_uint8(um_caw);
	r = CAW_get_uint8(um_caw);
	is_senderless = CAW_get_uint8(um_caw);
	if (is_senderless == 1) {
		printf("Ignoring senderless message\n");
		return;
	} else {
		CAW_set_pos(um_caw, CAW_get_pos_byte(um_caw) - 1, CAW_get_pos_bit(um_caw));

		chat = CAW_get_chars(um_caw, CAW_dist_until_null(um_caw));
		from = CAW_get_chars(um_caw, CAW_dist_until_null(um_caw));
		mesg = CAW_get_chars(um_caw, CAW_dist_until_null(um_caw));
		// printf("%u %u] in <", client, r, chat);
		// uint32_t i = 0;
		// while (1) {
		// 	printf("%x ", chat[i]);
		// 	if (chat[i] == 0) { break; }
		// 	i++;
		// }
		// printf(">: ");
		// i = 0;
		// while (1) {
		// 	printf("%x ", from[i]);
		// 	if (from[i] == 0) { break; }
		// 	i++;
		// }
		// printf(": ");
		// i = 0;
		// while (1) {
		// 	printf("%x ", mesg[i]);
		// 	if (mesg[i] == 0) { break; }
		// 	i++;
		// }
		// printf("\n");
	}
	message_json = cJSON_CreateObject();
	if (message_json == NULL) {
		parser_state->FAILURE |= 0b10000;
		return;
	}
	if (cJSON_AddNumberToObject(message_json, "tick", parser_state->tick) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "chat", chat) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "from", from) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "message", mesg) == NULL) { json_err = 1; }

	if (message_json == NULL) {
		parser_state->FAILURE |= 0b10000;
		return;
	}
	cJSON_AddItemToArray(json_chat_array, message_json);
}
