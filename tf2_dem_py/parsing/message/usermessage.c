
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/usermessage.h"

inline void handle_say_text(CharArrayWrapper *um_caw, ParserState *parser_state, cJSON *json_chat_array) {
	uint8_t client;
	uint8_t r;
	uint8_t is_senderless;
	char *chat = NULL;
	char *from = NULL;
	char *mesg = NULL;
	cJSON *message_json = NULL;
	uint8_t json_err = 0;

	client = CAW_get_uint8(um_caw);
	r = CAW_get_uint8(um_caw);
	is_senderless = CAW_get_uint8(um_caw);
	if (is_senderless == 1 || parser_state->current_message_contains_senderless_chat == 1) {
		// purely speculative and highly likely the false way to do this.
		parser_state->current_message_contains_senderless_chat = 1;
		return;
	}
	CAW_set_pos(um_caw, CAW_get_pos_byte(um_caw) - 1, CAW_get_pos_bit(um_caw));

	chat = (char *)CAW_get_nulltrm_str(um_caw);
	from = (char *)CAW_get_nulltrm_str(um_caw);
	mesg = (char *)CAW_get_nulltrm_str(um_caw);
	if (um_caw->ERRORLEVEL != 0) {
		return; // Will be taken care of by p_UserMessage.
	}

	// Store read chat message in json
	message_json = cJSON_CreateObject();
	if (message_json == NULL) {
		parser_state->FAILURE |= ERR.CJSON;
		return;
	}
	if (cJSON_AddNumberToObject(message_json, "tick", parser_state->tick) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "chat", chat) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "from", from) == NULL) { json_err = 1; }
	if (cJSON_AddVolatileStringRefToObject(message_json, "message", mesg) == NULL) { json_err = 1; }

	if (json_err == 1) {
		parser_state->FAILURE |= ERR.CJSON;
		return;
	}
	cJSON_AddItemToArray(json_chat_array, message_json);
}

void p_UserMessage(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint8_t user_message_type;
	uint16_t len = 0;

	user_message_type = CAW_get_uint8(caw);
	CAW_read_raw(caw, &len, 1, 3);
	// Calculate the length (in bytes) for the new CAW, the base CAW's bit
	// offset in mind.
	CharArrayWrapper *user_message_caw = CAW_from_caw_b(caw, len);
	if (user_message_caw == NULL) {
		parser_state->FAILURE |= ERR.MEMORY_ALLOCATION;
		return;
	}

	switch (user_message_type) {
	case 4:
		if (parser_state->flags & FLAGS.CHAT) {
			handle_say_text(user_message_caw, parser_state, cJSON_GetObjectItemCaseSensitive(root_json, "chat"));
		}
		break;
	default:
		break;
	}
	if (user_message_caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ERR.CAW;
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
