
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/parsing/helpers.hpp"
#include "tf2_dem_py/constants.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/message/usermessage.hpp"

using ParserState::ParserState_c;

// === UserMessage handlers, maybe outsource those idk ===

void handle_SayText(CharArrayWrapper *um_caw, ParserState_c parser_state, PyObject *chat_list) {

}

PyObject *handle_SayText2(CharArrayWrapper *um_caw, ParserState_c *parser_state) {
	PyObject *chat[9];
	uint8_t sender;
	uint8_t b_chat;
	PyObject *tmp0, *tmp1;
	PyObject *result = NULL;

	sender = um_caw->get_uint8();
	b_chat = um_caw->get_uint8(); //whatever that means

	// Debug string dumper (May cause memleak)
	 // PyObject *bytes, *strrep;
	 // size_t temp_by;
	 // uint8_t temp_bi;
	 // const char *str;
 
	 // temp_by = um_caw->get_pos_byte();
	 // temp_bi = um_caw->get_pos_bit();
	 // bytes = PyBytes_FromCAWLen(um_caw, um_caw->dist_until_null());
	 // if (bytes == NULL) {
	 // 	return;
	 // }
	 // strrep = PyObject_Repr(bytes);
	 // if (strrep == NULL) {
	 // 	return;
	 // }
	 // str = PyUnicode_AsUTF8(strrep);
	 // printf("Sender: % 3u; bChat: %u; %s\n", sender, b_chat, str);
	 // Py_DECREF(strrep); Py_DECREF(bytes);
	 // um_caw->set_pos(temp_by, temp_bi);
	// Debug string dumper end

	chat[1] = PyLong_FromLong(parser_state->tick); // Param0
	chat[2] = PyLong_FromLong(sender); // sender
	chat[3] = PyBool_FromLong(b_chat); // bChat

	if (um_caw->get_uint8() < 16) {
		// Strange message that doesn't conform to what SayText2 actually should be and only
		// contains a single string with formatting symbols sprinkled in

		Py_INCREF(Py_False);
		chat[0] = Py_False;
		chat[4] = NULL;

		tmp0 = PyBytes_FromCAWLen(um_caw, um_caw->dist_until_null());
		if (tmp0 == NULL) goto create_result;
		tmp1 = PyObject_Repr(tmp0);
		Py_DECREF(tmp0);

		if (tmp1 == NULL) goto create_result;
		chat[4] = PyUnicode_Substring(tmp1, 2, PyUnicode_GetLength(tmp1) - 1);
		Py_DECREF(tmp1);
		for (uint8_t i = 5; i < 9; i++) {
			Py_INCREF(Py_None);
			chat[i] = Py_None;
		}
	} else {
		um_caw->set_pos(um_caw->get_pos_byte() - 1, um_caw->get_pos_bit());

		Py_INCREF(Py_True);
		chat[0] = Py_True;
		chat[4] = PyUnicode_FromCAWNulltrm(um_caw); // Messagename / Channel
		chat[5] = PyUnicode_FromCAWNulltrm(um_caw); // Param1
		chat[6] = PyUnicode_FromCAWNulltrm(um_caw); // Param2
		chat[7] = PyUnicode_FromCAWNulltrm(um_caw); // Param3
		chat[8] = PyUnicode_FromCAWNulltrm(um_caw); // Param4
	}
	create_result:

	if (parser_state->flags & FLAGS::COMPACT_CHAT) {
		result = PyTuple_FromArrayTransfer(9, chat);
		if (result == NULL) {
			parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		}
	} else {
		result = CreateDict_Strings(CONSTANTS::DICT_NAMES_SayText2->py_strings, chat, 9);
		if (result == NULL) {
			parser_state->FAILURE |= (ParserState::ERRORS::MEMORY_ALLOCATION | ParserState::ERRORS::PYDICT);
		}
	}

	return result;
}

// === End of UserMessage handlers ===

namespace MessageParsers {

void UserMessage::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	uint8_t user_message_type;
	uint16_t len = 0;
	PyObject *parsed_message;
	PyObject *message_list;

	user_message_type = caw->get_uint8();
	caw->read_raw(&len, 1, 3);
	CharArrayWrapper *user_message_caw = caw->caw_from_caw_b(len);
	if (user_message_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	switch (user_message_type) {
		case 4:
			if (parser_state->flags & FLAGS::CHAT) {
				parsed_message = handle_SayText2(user_message_caw, parser_state);

				message_list = parser_state->chat_container;
				if (parser_state->flags & FLAGS::COMPACT_CHAT) {
					message_list = PyTuple_GetItem(message_list, 1);
				}

				if (PyList_Append(message_list, parsed_message) < 0) {
					Py_DECREF(parsed_message);
					parser_state->FAILURE |= ParserState::ERRORS::PYLIST;
					goto cleanup_and_ret;
				}
				Py_DECREF(parsed_message);

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
