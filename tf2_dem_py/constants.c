#ifndef NO_PYTHON
#  define PY_SSIZE_T_CLEAN
#  include <Python.h>
#endif

#include "tf2_dem_py/constants.h"

#ifndef NO_PYTHON
PyStringHolder *PyStringHolder_new(const char **strings, Py_ssize_t size) {
	uint8_t failed = 0;

	PyStringHolder *self = (PyStringHolder *)malloc(sizeof(PyStringHolder));
	if (self == NULL) {
		goto fail0;
	}

	self->size = size;
	self->py_strings = NULL;

	self->py_strings = (PyObject **)malloc(sizeof(PyObject *) * size);
	for (Py_ssize_t i = 0; i < size; i++) {
		self->py_strings[i] = PyUnicode_FromString(strings[i]);
		if (self->py_strings[i] == NULL) {
			failed = 1;
			Py_INCREF(Py_None);
			self->py_strings[i] = Py_None;
		}
	}
	if (failed) {
		goto fail1;
	}
	return self;

fail1: free(self);
fail0:
	return NULL;
}

void PyStringHolder_destroy(PyStringHolder *self) {
	if (self->py_strings != NULL) {
		for (Py_ssize_t i = 0; i < self->size; i++) {
			Py_DECREF(self->py_strings[i]);
		}
		free(self->py_strings);
	}
	free(self);
}

PyObject *PyStringHolder_getPyTuple(PyStringHolder *self) {
	PyObject *tuple = PyTuple_New(self->size);
	if (tuple == NULL) {
		return NULL;
	}
	for (Py_ssize_t i = 0; i < self->size; i++) {
		Py_INCREF(self->py_strings[i]);
		PyTuple_SET_ITEM(tuple, i, self->py_strings[i]);
	}
	return tuple;
}

// === Python constants === //
// DO NOT USE THEM IN THIS STATE, call CONSTANTS_initialize first!

PyStringHolder
	*CONSTANTS_DICT_NAMES_ChatMessage = NULL,
	*CONSTANTS_DICT_NAMES_GameEventContainer = NULL,
	*CONSTANTS_DICT_NAMES_DemoHeader = NULL,
	*CONSTANTS_DICT_NAMES_ServerInfo = NULL;

int CONSTANTS_initialize() {
	// Constant setup
	const char *_arr_SayText2[8] = {
		"is_normal", "sender", "is_chat", "data", "param1", "param2", "param3", "param4"
	};
	const char *_arr_GameEventContainer[3] = {"name", "field_names", "data"};
	const char *_arr_DemoHeader[11] = {
		"ident", "net_prot", "dem_prot", "host_addr", "client_id", "map_name",
		"game_dir", "play_time", "tick_count", "frame_count", "sigon"
	};
	const char *_arr_ServerInfo[16] = {
		"version", "server_count", "stv", "dedicated", "max_crc", "max_classes",
		"map_hash", "player_count", "max_player_count", "interval_per_tick", "platform",
		"game", "map_name", "skybox", "server_name", "replay",
	};

	// Setting StringHolders
	CONSTANTS_DICT_NAMES_ChatMessage = PyStringHolder_new(_arr_SayText2, 8);
	CONSTANTS_DICT_NAMES_GameEventContainer = PyStringHolder_new(_arr_GameEventContainer, 3);
	CONSTANTS_DICT_NAMES_DemoHeader = PyStringHolder_new(_arr_DemoHeader, 11);
	CONSTANTS_DICT_NAMES_ServerInfo = PyStringHolder_new(_arr_ServerInfo, 16);

	// Checking StringHolders
	if (
		CONSTANTS_DICT_NAMES_ChatMessage == NULL || CONSTANTS_DICT_NAMES_GameEventContainer == NULL ||
		CONSTANTS_DICT_NAMES_DemoHeader == NULL || CONSTANTS_DICT_NAMES_ServerInfo == NULL
	) { return 1; }

	return 0;
}

void CONSTANTS_deallocate() {
	if (CONSTANTS_DICT_NAMES_ChatMessage != NULL) { PyStringHolder_destroy(CONSTANTS_DICT_NAMES_ChatMessage); }
	if (CONSTANTS_DICT_NAMES_GameEventContainer != NULL) { PyStringHolder_destroy(CONSTANTS_DICT_NAMES_GameEventContainer); }
	if (CONSTANTS_DICT_NAMES_DemoHeader != NULL) { PyStringHolder_destroy(CONSTANTS_DICT_NAMES_DemoHeader); }
	if (CONSTANTS_DICT_NAMES_ServerInfo != NULL) { PyStringHolder_destroy(CONSTANTS_DICT_NAMES_ServerInfo); }
}
#endif
