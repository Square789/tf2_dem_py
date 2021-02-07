#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/constants.h"

PyStringHolder *PyStringHolder_new(const char **strings, Py_ssize_t size) {
	uint8_t failed = 0;

	PyStringHolder *self = (PyStringHolder *)malloc(sizeof(PyStringHolder));

	self->strings = strings;
	self->size = size;
	self->py_strings = NULL;

	self->py_strings = (PyObject **)malloc(sizeof(PyObject *) * size);
	for (Py_ssize_t i = 0; i < size; i++) {
		self->py_strings[i] = PyUnicode_FromString(self->strings[i]);
		if (self->py_strings[i] == NULL) {
			failed = 1;
			Py_INCREF(Py_None);
			self->py_strings[i] = Py_None;
		}
	}
	if (failed) { return NULL; }
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

// === Simple constants === //

Py_ssize_t CONSTANTS_COMPACT_TUPLE2_FIELD_NAMES_IDX = 0;
Py_ssize_t CONSTANTS_COMPACT_TUPLE2_DATA_IDX = 1;
Py_ssize_t CONSTANTS_COMPACT_TUPLE3_FIELD_NAMES_IDX = 1;
Py_ssize_t CONSTANTS_COMPACT_TUPLE3_NAME_IDX = 0;
Py_ssize_t CONSTANTS_COMPACT_TUPLE3_DATA_IDX = 2;

// === Python constants === //
// DO NOT USE THEM IN THIS STATE, call CONSTANTS_initialize first!
PyObject
	*CONSTANTS_PYSTR_FIELD_NAMES = NULL,
	*CONSTANTS_PYSTR_DATA = NULL,
	*CONSTANTS_PYSTR_NAME = NULL;
PyStringHolder
	*CONSTANTS_DICT_NAMES_SayText2 = NULL,
	*CONSTANTS_DICT_NAMES_GameEvent = NULL,
	*CONSTANTS_DICT_NAMES_header = NULL;

int CONSTANTS_initialize() {
	// Constant setup
	const char *_arr_SayText2[9] = {"normal (params non-None)", "tick", "sender_entidx",
		"bChat", "normal?channel:message", "param1", "param2", "param3", "param4"};
	const char *_arr_GameEvent[3] = {"name", "field_names", "data"};
	const char *_arr_header[11] = {"ident", "net_prot", "dem_prot", "host_addr",
		"client_id", "map_name", "game_dir", "play_time", "tick_count",
		"frame_count", "sigon"};

	// Setting solo strings
	CONSTANTS_PYSTR_FIELD_NAMES = PyUnicode_FromStringAndSize("field_names", 11);
	CONSTANTS_PYSTR_DATA = PyUnicode_FromStringAndSize("data", 4);
	CONSTANTS_PYSTR_NAME = PyUnicode_FromStringAndSize("name", 4);

	// Checking solo strings
	if (
		CONSTANTS_PYSTR_FIELD_NAMES == NULL ||
		CONSTANTS_PYSTR_DATA == NULL ||
		CONSTANTS_PYSTR_NAME == NULL
	) { return -1; }

	// Setting StringHolders
	CONSTANTS_DICT_NAMES_SayText2 = PyStringHolder_new(_arr_SayText2, 9);
	CONSTANTS_DICT_NAMES_GameEvent = PyStringHolder_new(_arr_GameEvent, 3);
	CONSTANTS_DICT_NAMES_header = PyStringHolder_new(_arr_header, 11);

	// Checking solo strings
	if (
		CONSTANTS_DICT_NAMES_SayText2 == NULL ||
		CONSTANTS_DICT_NAMES_GameEvent == NULL ||
		CONSTANTS_DICT_NAMES_header == NULL
	) { return -1; }

	return 0;
}

void CONSTANTS__deallocate_stringholders() {
	PyStringHolder_destroy(CONSTANTS_DICT_NAMES_SayText2);
	PyStringHolder_destroy(CONSTANTS_DICT_NAMES_GameEvent);
	PyStringHolder_destroy(CONSTANTS_DICT_NAMES_header);
}

void CONSTANTS_deallocate() {
	Py_DECREF(CONSTANTS_PYSTR_FIELD_NAMES);
	Py_DECREF(CONSTANTS_PYSTR_DATA);
	Py_DECREF(CONSTANTS_PYSTR_NAME);
	CONSTANTS__deallocate_stringholders();
}

void CONSTANTS_deallocate_safe() {
	Py_XDECREF(CONSTANTS_PYSTR_FIELD_NAMES);
	Py_XDECREF(CONSTANTS_PYSTR_DATA);
	Py_XDECREF(CONSTANTS_PYSTR_NAME);
	CONSTANTS__deallocate_stringholders();
}

