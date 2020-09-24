
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <new>

#include "tf2_dem_py/constants.hpp"

namespace CONSTANTS {

PyStringHolder::PyStringHolder(const char **strings, Py_ssize_t size) {
	uint8_t failed = 0;
	this->strings = strings;
	this->size = size;
	this->py_strings = NULL;

	this->py_strings = new PyObject*[size];
	for (Py_ssize_t i = 0; i < size; i++) {
		this->py_strings[i] = PyUnicode_FromString(this->strings[i]);
		if (this->py_strings[i] == NULL) {
			failed = 1;
			Py_INCREF(Py_None);
			this->py_strings[i] = Py_None;
		}
	}
	if (failed) throw std::bad_alloc();
}

PyStringHolder::~PyStringHolder() {
	//if (this->strings != NULL) {
	//	delete[] this->strings;
	//}
	if (this->py_strings != NULL) {
		for (Py_ssize_t i = 0; i < this->size; i++) {
			Py_DECREF(this->py_strings[i]);
		}
		delete[] this->py_strings;
	}
}

PyObject *PyStringHolder::create_PyTuple() {
	PyObject *tuple = PyTuple_New(this->size);
	if (tuple == NULL) {
		return NULL;
	}
	for (Py_ssize_t i = 0; i < this->size; i++) {
		Py_INCREF(this->py_strings[i]);
		PyTuple_SET_ITEM(tuple, i, this->py_strings[i]);
	}
	return tuple;
}

uint16_t MAX_GAME_EVENT_AMOUNT = 1 << 9;
Py_ssize_t COMPACT_TUPLE2_FIELD_NAMES_IDX = 0;
Py_ssize_t COMPACT_TUPLE2_DATA_IDX = 1;
Py_ssize_t COMPACT_TUPLE3_FIELD_NAMES_IDX = 1;
Py_ssize_t COMPACT_TUPLE3_NAME_IDX = 0;
Py_ssize_t COMPACT_TUPLE3_DATA_IDX = 2;

// Set to NULL so the linker doesn't complain and for safety of delete; DO NOT USE THEM IN THIS STATE
PyObject *PYSTR_FIELD_NAMES = NULL, *PYSTR_DATA = NULL, *PYSTR_NAME = NULL;
PyStringHolder *DICT_NAMES_SayText2 = NULL, *DICT_NAMES_GameEvent = NULL, *DICT_NAMES_header = NULL;

int initialize() {
	// Constant setup
	const char *_arr_SayText2[9] = {"normal (params non-None)", "tick", "sender_entidx",
		"bChat", "normal?channel:message", "param1", "param2", "param3", "param4"};
	const char *_arr_GameEvent[3] = {"name", "field_names", "data"};
	const char *_arr_header[11] = {"ident", "net_prot", "dem_prot", "host_addr",
		"client_id", "map_name", "game_dir", "play_time", "tick_count",
		"frame_count", "sigon"};

	// Setting solo strings
	PYSTR_FIELD_NAMES = PyUnicode_FromStringAndSize("field_names", 11);
	PYSTR_DATA = PyUnicode_FromStringAndSize("data", 4);
	PYSTR_NAME = PyUnicode_FromStringAndSize("name", 4);

	// Checking solo strings
	if (PYSTR_FIELD_NAMES == NULL || PYSTR_DATA == NULL || PYSTR_NAME == NULL) {
		return -1;
	}

	// Setting StringHolders
	try {
		DICT_NAMES_SayText2 = new PyStringHolder(_arr_SayText2, 9);
		DICT_NAMES_GameEvent = new PyStringHolder(_arr_GameEvent, 3);
		DICT_NAMES_header = new PyStringHolder(_arr_header, 11);
	} catch (std::bad_alloc& ba) {
		return -1;
	}

	return 0;
}

void _deallocate_stringholders() {
	delete DICT_NAMES_SayText2;
	delete DICT_NAMES_GameEvent;
	delete DICT_NAMES_header;
}

void deallocate() {
	Py_DECREF(PYSTR_FIELD_NAMES);
	Py_DECREF(PYSTR_DATA);
	Py_DECREF(PYSTR_NAME);
	_deallocate_stringholders();
}

void deallocate_safe() {
	Py_XDECREF(PYSTR_FIELD_NAMES);
	Py_XDECREF(PYSTR_DATA);
	Py_XDECREF(PYSTR_NAME);
	_deallocate_stringholders();
}

}
