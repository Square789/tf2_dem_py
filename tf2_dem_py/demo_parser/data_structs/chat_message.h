#ifndef CHAT_MESSAGE__H
#define CHAT_MESSAGE__H

#include <stdbool.h>

#define PY_SSIZE_T_CLEAN
#include "Python.h"

typedef struct {
	bool is_normal;
	uint8_t sender;
	bool is_chat;
	uint8_t *data;
	char *param0;
	char *param1;
	char *param2;
	char *param3;
} ChatMessage;

ChatMessage *ChatMessage_new();
void ChatMessage_init(ChatMessage *self);
void ChatMessage_destroy(ChatMessage *self);

PyObject *ChatMessage_to_PyDict(ChatMessage *self);
PyObject *ChatMessage_to_PyTuple(ChatMessage *self);

#endif
