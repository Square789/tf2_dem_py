#include <stdint.h>
#include <math.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/message/usermessage.h"

// === UserMessage handlers, maybe outsource those idk ===

void handle_SayText(CharArrayWrapper *um_caw, ParserState parser_state, PyObject *chat_list) {

}

PyObject *handle_SayText2(CharArrayWrapper *um_caw, ParserState *parser_state) {
	PyObject *chat[9];
	uint8_t sender;
	uint8_t b_chat;
	PyObject *tmp0, *tmp1;
	PyObject *result = NULL;

	sender = CharArrayWrapper_get_uint8(um_caw);
	b_chat = CharArrayWrapper_get_uint8(um_caw); //whatever that means

	// Debug string dumper (Will cause memleak)
	 // PyObject *bytes, *strrep;
	 // size_t temp_by;
	 // uint8_t temp_bi;
	 // const char *str;
 
	 // temp_by = CharArrayWrapper_get_pos_byte(um_caw);
	 // temp_bi = CharArrayWrapper_get_pos_bit(um_caw);
	 // bytes = PyBytes_FromCAWLen(um_caw, CharArrayWrapper_dist_until_null(um_caw));
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
	 // CharArrayWrapper_set_pos(um_caw, temp_by, temp_bi);
	// Debug string dumper end

	chat[1] = PyLong_FromLong(parser_state->tick); // Param0
	chat[2] = PyLong_FromLong(sender); // sender
	chat[3] = PyBool_FromLong(b_chat); // bChat

	if (CharArrayWrapper_get_uint8(um_caw) < 16) {
		// Strange message that doesn't conform to what SayText2 actually should be and only
		// contains a single string with formatting symbols sprinkled in

		Py_INCREF(Py_False);
		chat[0] = Py_False;
		chat[4] = NULL;

		tmp0 = PyBytes_FromCAWLen(um_caw, CharArrayWrapper_dist_until_null(um_caw));
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
		CharArrayWrapper_set_pos(
			um_caw,
			CharArrayWrapper_get_pos_byte(um_caw) - 1,
			CharArrayWrapper_get_pos_bit(um_caw)
		);

		Py_INCREF(Py_True);
		chat[0] = Py_True;
		chat[4] = PyUnicode_FromCAWNulltrm(um_caw); // Messagename / Channel
		chat[5] = PyUnicode_FromCAWNulltrm(um_caw); // Param1
		chat[6] = PyUnicode_FromCAWNulltrm(um_caw); // Param2
		chat[7] = PyUnicode_FromCAWNulltrm(um_caw); // Param3
		chat[8] = PyUnicode_FromCAWNulltrm(um_caw); // Param4
	}
	create_result:

	if (parser_state->flags & FLAGS_COMPACT_CHAT) {
		result = PyTuple_FromArrayTransfer(9, chat);
		if (result == NULL) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		}
	} else {
		result = CreateDict_Strings(CONSTANTS_DICT_NAMES_SayText2->py_strings, chat, 9);
		if (result == NULL) {
			parser_state->failure |= (ParserState_ERR_MEMORY_ALLOCATION | ParserState_ERR_PYDICT);
		}
	}

	return result;
}

// === End of UserMessage handlers ===

void UserMessage_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	// Temporary garbage solution
	UserMessage_skip(caw, parser_state);
// 	uint8_t user_message_type;
// 	uint16_t len = 0;
// 	PyObject *parsed_message;
// 	PyObject *message_list;
// 	CharArrayWrapper *user_message_caw;

// 	user_message_type = CharArrayWrapper_get_uint8(caw);
// 	CharArrayWrapper_read_raw(caw, &len, 1, 3);
// 	user_message_caw = CharArrayWrapper_from_caw_b(caw, len);
// 	if (user_message_caw == NULL) {
// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
// 		return;
// 	}

// 	switch (user_message_type) {
// 		case 4:
// 			if (parser_state->flags & FLAGS_CHAT) {
// 				parsed_message = handle_SayText2(user_message_caw, parser_state);

// 				message_list = parser_state->chat_container;
// 				if (parser_state->flags & FLAGS_COMPACT_CHAT) {
// 					message_list = PyTuple_GetItem(message_list, 1);
// 				}

// 				if (PyList_Append(message_list, parsed_message) < 0) {
// 					Py_DECREF(parsed_message);
// 					parser_state->failure |= ParserState_ERR_PYLIST;
// 					goto cleanup_and_ret;
// 				}
// 				Py_DECREF(parsed_message);

// 			}
// 			break;
// 		default:
// 			break;
// 	}
// 	if (user_message_caw->ERRORLEVEL != 0) {
// 		parser_state->failure |= ParserState_ERR_CAW;
// 		parser_state->RELAYED_CAW_ERR = user_message_caw->ERRORLEVEL;
// 		// This may be ambigous in really odd cases where there is a message
// 		// length discrepancy and the inner caw sets the out of bounds flag.
// 	}
// cleanup_and_ret:
// 	CharArrayWrapper_destroy(user_message_caw);
}

void UserMessage_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 0);
	uint16_t len;
	CharArrayWrapper_read_raw(caw, &len, 1, 3);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}
