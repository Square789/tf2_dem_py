
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdlib.h>
#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/helpers.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/message/usermessage.hpp"

using ParserState::ParserState_c;

namespace MessageParsers {

inline void handle_say_text(CharArrayWrapper *um_caw, ParserState_c *parser_state, PyObject *chat_list) {
	static const char *CHAT_DICT_NAMES[] = {"tick", "chat", "from", "message"};
	uint8_t client;
	uint8_t r;
	uint8_t is_senderless;
	PyObject *message_dict;
	PyObject *chat[4];

	client = um_caw->get_uint8();
	r = um_caw->get_uint8();
	is_senderless = um_caw->get_uint8();
	if (is_senderless == 1 || parser_state->current_message_contains_senderless_chat == 1) {
		// purely speculative and highly likely the false way to do this.
		parser_state->current_message_contains_senderless_chat = 1;
		return;
	}
	um_caw->set_pos(um_caw->get_pos_byte() - 1, um_caw->get_pos_bit());

	// Store read chat message in json
	message_dict = PyDict_New();
	if (message_dict == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	chat[0] = PyLong_FromLong(parser_state->tick);
	chat[1] = PyUnicode_FromCAWNulltrm(um_caw);
	chat[2] = PyUnicode_FromCAWNulltrm(um_caw);
	chat[3] = PyUnicode_FromCAWNulltrm(um_caw);
	if (um_caw->ERRORLEVEL != 0) {
		return; // Will be taken care of by p_UserMessage.
	}
	for (uint8_t i = 0; i < 4; i++) {
		if (chat[i] == NULL) { // Python conversion failure, error raised already
			parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		} else {
			if (PyDict_SetItemString(message_dict, CHAT_DICT_NAMES[i], chat[i]) < 0) {
				parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
			} // Move value into dict, then decrease refcount
			Py_DECREF(chat[i]);
		}
	}

	PyList_Append(chat_list, message_dict);
}

void UserMessage::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	uint8_t user_message_type;
	uint16_t len = 0;
	PyObject *message_list;

	user_message_type = caw->get_uint8();
	caw->read_raw(&len, 1, 3);
	// Calculate the length (in bytes) for the new CAW, the base CAW's bit
	// offset in mind.
	CharArrayWrapper *user_message_caw = caw->caw_from_caw_b(len);
	if (user_message_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	switch (user_message_type) {
		case 4:
			if (parser_state->flags & FLAGS.CHAT) {
				message_list = PyDict_GetItemString(root_dict, "chat");
				if (message_list == NULL) {
					parser_state->FAILURE |= ParserState::ERRORS::PYLIST;
					goto cleanup_and_ret;
				}
				handle_say_text(user_message_caw, parser_state, message_list);
			}
			break;
		default:
			break;
	}
	if (user_message_caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ParserState::ERRORS::CAW;
		parser_state->RELAYED_CAW_ERR = user_message_caw->ERRORLEVEL;
		// This may be ambigous in really odd cases where there is a message
		// length discrepancy and the inner caw sets the out of bounds flag.
	}
cleanup_and_ret:
	delete user_message_caw;
}

void UserMessage::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	caw->skip(1, 0);
	uint16_t len;
	caw->read_raw(&len, 1, 3);
	caw->skip(len / 8, len % 8);
}

}
