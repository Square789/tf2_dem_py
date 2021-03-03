
#include <stdbool.h>
#include <stdio.h>

#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/data_structs/chat_message.h"

ChatMessage *ChatMessage_new() {
	ChatMessage *self = (ChatMessage *)malloc(sizeof(ChatMessage));
	if (self == NULL) {
		return NULL;
	}
	ChatMessage_init(self);
	return self;
}

void ChatMessage_init(ChatMessage *self) {
	self->is_normal = false;
	self->sender = 0;
	self->is_chat = false;
	self->data = NULL;
	self->param0 = NULL;
	self->param1 = NULL;
	self->param2 = NULL;
	self->param3 = NULL;
}

void ChatMessage_destroy(ChatMessage *self) {
	if (self->data    != NULL) { free(self->data);    }
	if (self->param0  != NULL) { free(self->param0);  }
	if (self->param1  != NULL) { free(self->param1);  }
	if (self->param2  != NULL) { free(self->param2);  }
	if (self->param3  != NULL) { free(self->param3);  }
	free(self);
}

PyObject *ChatMessage_to_PyDict(ChatMessage *self) {
	PyObject *chat_dict = PyDict_New();
	PyObject *chat[8];
	bool failed = false;
	printf("\n");

	if (chat_dict == NULL) {
		return NULL;
	}

	chat[0] = PyBool_FromLong(self->is_normal);
	chat[1] = PyLong_FromUnsignedLong(self->sender);
	chat[2] = PyBool_FromLong(self->is_chat);
	chat[3] = byte_repr_from_chars(self->data);
	chat[4] = byte_repr_from_chars(self->param0);
	chat[5] = byte_repr_from_chars(self->param1);
	chat[6] = byte_repr_from_chars(self->param2);
	chat[7] = byte_repr_from_chars(self->param3);

	for (size_t i = 0; i < 8; i++) {
		if (chat[i] == NULL) {
			failed = true;
			continue;
		}
		if (PyDict_SetItem(chat_dict, CONSTANTS_DICT_NAMES_ChatMessage->py_strings[i], chat[i]) < 0) {
			failed = true;
		}
		Py_DECREF(chat[i]);
	}

	if (failed) {
		Py_DECREF(chat_dict);
		return NULL;
	}

	return chat_dict;
}

PyObject *ChatMessage_to_PyTuple(ChatMessage *self) {
	PyObject *tup = PyTuple_New(8);
	if (tup == NULL) {
		return NULL;
	}

	PyTuple_SET_ITEM(tup, 0, PyBool_FromLong(self->is_normal));
	PyTuple_SET_ITEM(tup, 1, PyLong_FromUnsignedLong(self->sender));
	PyTuple_SET_ITEM(tup, 2, PyBool_FromLong(self->is_chat));
	PyTuple_SET_ITEM(tup, 3, byte_repr_from_chars(self->data));
	PyTuple_SET_ITEM(tup, 4, byte_repr_from_chars(self->param0));
	PyTuple_SET_ITEM(tup, 5, byte_repr_from_chars(self->param1));
	PyTuple_SET_ITEM(tup, 6, byte_repr_from_chars(self->param2));
	PyTuple_SET_ITEM(tup, 7, byte_repr_from_chars(self->param3));

	for (size_t i = 0; i < 8; i++) {
		if (PyTuple_GET_ITEM(tup, i) == NULL) {
			Py_DECREF(tup);
			return NULL;
		}
	}

	return tup;
}
