
#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/usermessage.hpp"

namespace MessageParsers {

inline void handle_say_text(CharArrayWrapper *um_caw, ParserState *parser_state, cJSON *json_chat_array) {
	uint8_t client;
	uint8_t r;
	uint8_t is_senderless;
	char *chat = NULL;
	char *from = NULL;
	char *mesg = NULL;
	cJSON *message_json = NULL;
	uint8_t json_err = 0;

	client = um_caw->get_uint8();
	r = um_caw->get_uint8();
	is_senderless = um_caw->get_uint8();
	if (is_senderless == 1 || parser_state->current_message_contains_senderless_chat == 1) {
		// purely speculative and highly likely the false way to do this.
		parser_state->current_message_contains_senderless_chat = 1;
		return;
	}
	um_caw->set_pos(um_caw->get_pos_byte() - 1, um_caw->get_pos_bit());

	chat = (char *)um_caw->get_nulltrm_str();
	from = (char *)um_caw->get_nulltrm_str();
	mesg = (char *)um_caw->get_nulltrm_str();
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

void UserMessage::parse(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint8_t user_message_type;
	uint16_t len = 0;

	user_message_type = caw->get_uint8();
	caw->read_raw(&len, 1, 3);
	// Calculate the length (in bytes) for the new CAW, the base CAW's bit
	// offset in mind.
	CharArrayWrapper *user_message_caw = caw->caw_from_caw_b(len);
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
	delete user_message_caw;
}

void UserMessage::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 0);
	uint16_t len;
	caw->read_raw(&len, 1, 3);
	caw->skip(len / 8, len % 8);
}

}
