#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/demo_parser/parser_state.h"

#include "tf2_dem_py/demo_parser/message/usermessage.h"

// === UserMessage handlers, maybe outsource those idk ===

// Returns:
// 0 on success
// 1 on CharArrayWrapper failure (may also be related to memory problems)
// 2 on memory allocation / ChatMessage insertion failure
uint8_t handle_SayText2(CharArrayWrapper *um_caw, ParserState *parser_state, CharArrayWrapper_err_t *caw_err) {
	ChatMessage *extracted_chat_message;

	extracted_chat_message = ChatMessage_new();
	if (extracted_chat_message == NULL) {
		return 2;
	}

	extracted_chat_message->sender = CharArrayWrapper_get_uint8(um_caw);;
	extracted_chat_message->is_chat = CharArrayWrapper_get_uint8(um_caw);;

	if (CharArrayWrapper_get_uint8(um_caw) < 16) {
		// Strange message that doesn't conform to what SayText2 actually should be and only
		// contains a single string with formatting symbols sprinkled in

		extracted_chat_message->is_normal = false;
		extracted_chat_message->data = CharArrayWrapper_get_nulltrm_str(um_caw);
	} else {
		// Rewind since then the initial byte is actually part of the message
		CharArrayWrapper_set_pos(
			um_caw,
			CharArrayWrapper_get_pos_byte(um_caw) - 1,
			CharArrayWrapper_get_pos_bit(um_caw)
		);

		extracted_chat_message->is_normal = true;
		extracted_chat_message->data = CharArrayWrapper_get_nulltrm_str(um_caw);
		extracted_chat_message->param0 = CharArrayWrapper_get_nulltrm_str(um_caw);
		extracted_chat_message->param1 = CharArrayWrapper_get_nulltrm_str(um_caw);
		extracted_chat_message->param2 = CharArrayWrapper_get_nulltrm_str(um_caw);
		extracted_chat_message->param3 = CharArrayWrapper_get_nulltrm_str(um_caw);
	}

	if (um_caw->error != 0) {
		ChatMessage_destroy(extracted_chat_message);
		*caw_err = um_caw->error;
		return 1;
	}

	if (ParserState_append_chat_message(parser_state, extracted_chat_message) > 0) {
		ChatMessage_destroy(extracted_chat_message);
		return 2;
	}

	return 0;
}

// === End of UserMessage handlers ===

void UserMessage_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t user_message_type;
	uint16_t len = 0;
	CharArrayWrapper *user_message_caw;

	CharArrayWrapper_err_t caw_err;

	user_message_type = CharArrayWrapper_get_uint8(caw);
	CharArrayWrapper_read_raw(caw, &len, 1, 3);
	user_message_caw = CharArrayWrapper_from_caw_b(caw, len);
	if (user_message_caw == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return;
	}

	// There is massive room for improving this
	switch (user_message_type) {
	case 4:
		if (!(parser_state->flags & FLAGS_CHAT)) {
			break;
		}
		switch (handle_SayText2(user_message_caw, parser_state, &caw_err)) {
		case 1:
			if (caw_err & CAW_ERR_MEMORY_ALLOCATION) {
				parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			}
			parser_state->failure |= ParserState_ERR_CAW;
			parser_state->relayed_caw_error = caw_err;
			break;
		case 2:
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			break;
		default:
			break;
		}
		break;
	default:
		break;
	}
	if (user_message_caw->error != 0) {
		// This may be ambigous in really odd cases where there is a message
		// length discrepancy and the inner caw sets the out of bounds flag.
		parser_state->failure |= ParserState_ERR_CAW;
		parser_state->relayed_caw_error = user_message_caw->error;
	}

	CharArrayWrapper_destroy(user_message_caw);
}

void UserMessage_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CharArrayWrapper_skip(caw, 1, 0);
	CharArrayWrapper_read_raw(caw, &len, 1, 3);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}
