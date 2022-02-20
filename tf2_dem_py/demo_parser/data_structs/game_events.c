#include <stdint.h>
#include <stdlib.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/data_structs/game_events.h"

// === GameEventEntry

GameEventEntry *GameEventEntry_new() {
	GameEventEntry *self = (GameEventEntry *)malloc(sizeof(GameEventEntry));
	if (self == NULL) {
		return NULL;
	}
	GameEventEntry_init(self);
	return self;
}

void GameEventEntry_init(GameEventEntry *self) {
	self->name = NULL;
	self->type = 0xFF;
#ifndef NO_PYTHON
	self->py_name = NULL;
#endif
}

void GameEventEntry_free(GameEventEntry *self) {
	free(self->name);
#ifndef NO_PYTHON
	Py_XDECREF(self->py_name);
#endif
}

void GameEventEntry_destroy(GameEventEntry *self) {
	GameEventEntry_free(self);
	free(self);
}

#ifndef NO_PYTHON
PyObject *GameEventEntry_get_python_name(GameEventEntry *self) {
	if (self->py_name == NULL) {
		self->py_name = PyUnicode_FromString(self->name);
		if (self->py_name == NULL) {
			return NULL;
		}
	}
	Py_INCREF(self->py_name);
	return self->py_name;
}
#endif


// === GameEvent

GameEvent *GameEvent_new() {
	GameEvent *self = (GameEvent *)malloc(sizeof(GameEvent));
	if (self == NULL) {
		return NULL;
	}
	GameEvent_init(self);
	return self;
}

void GameEvent_init(GameEvent *self) {
	self->data = NULL;
	self->data_len = 0;
	self->event_type = 0;
}

void GameEvent_free(GameEvent *self) {
	free(self->data);
}

void GameEvent_destroy(GameEvent *self) {
	GameEvent_free(self);
	free(self);
}

#ifndef NO_PYTHON
PyObject *GameEvent_to_PyDict(GameEvent *self, GameEventDefinition *event_def) {
	PyObject *tmp_entry_val;
	PyObject *entry_dict;
	PyObject *entry_name;

	CharArrayWrapper *ge_caw;

	ge_caw = CharArrayWrapper_new();
	if (ge_caw == NULL) {
		goto error0;
	}
	ge_caw->mem_ptr = self->data;
	ge_caw->mem_len = self->data_len;
	ge_caw->free_on_dealloc = 0;

	entry_dict = PyDict_New();
	if (entry_dict == NULL) {
		goto error1;
	}

	// Fill game_event dict
	for (uint16_t i = 0; i < event_def->entries_len; i++) {
		switch (event_def->entries[i].type) {
		case 0: case 7: default: // 0 / anything > 7 should not be here, no idea what to do for 7
			tmp_entry_val = NULL;
			break;
		case 1: // String
			tmp_entry_val = PyUnicode_FromCAWNulltrm(ge_caw); break;
		case 2: // Float
			tmp_entry_val = PyFloat_FromDouble(CharArrayWrapper_get_flt(ge_caw)); break;
		case 3: // 32bInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int32(ge_caw)); break;
		case 4: // 16bInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int16(ge_caw)); break;
		case 5: // 8buInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_uint8(ge_caw)); break;
		case 6: // Bit
			tmp_entry_val = PyBool_FromLong(CharArrayWrapper_get_bit(ge_caw)); break;
		}
		if (tmp_entry_val == NULL) {
			goto error2;
		}

		entry_name = GameEventEntry_get_python_name(event_def->entries + i);
		if (entry_name == NULL) {
			Py_DECREF(tmp_entry_val);
			goto error2;
		}
		Py_DECREF(entry_name); // PyDict_SetItem increfs on its own, discard this reference.
		if (PyDict_SetItem(entry_dict, entry_name, tmp_entry_val) < 0) {
			Py_DECREF(tmp_entry_val);
			goto error2;
		}
		Py_DECREF(tmp_entry_val);
	}
	CharArrayWrapper_destroy(ge_caw);
	return entry_dict;

error2: Py_DECREF(entry_dict);
error1: CharArrayWrapper_destroy(ge_caw);
error0:
	return NULL;
}

PyObject *GameEvent_to_compact_PyTuple(GameEvent *self, GameEventDefinition *event_def) {
	PyObject *tmp_entry_val;
	PyObject *event_tup;

	CharArrayWrapper *ge_caw;

	ge_caw = CharArrayWrapper_new();
	if (ge_caw == NULL) {
		goto error0;
	}
	ge_caw->mem_ptr = self->data;
	ge_caw->mem_len = self->data_len;
	ge_caw->free_on_dealloc = 0;
	event_tup = PyTuple_New(event_def->entries_len);
	if (event_tup == NULL) {
		goto error1;
	}

	// Fill tuple
	for (uint16_t i = 0; i < event_def->entries_len; i++) {
		switch (event_def->entries[i].type) {
		case 0: case 7: default: // 0 / anything > 7 should not be here, no idea what to do for 7
			tmp_entry_val = NULL;
			break;
		case 1: // String
			tmp_entry_val = PyUnicode_FromCAWNulltrm(ge_caw); break;
		case 2: // Float
			tmp_entry_val = PyFloat_FromDouble(CharArrayWrapper_get_flt(ge_caw)); break;
		case 3: // 32bInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int32(ge_caw)); break;
		case 4: // 16bInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int16(ge_caw)); break;
		case 5: // 8buInteger
			tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_uint8(ge_caw)); break;
		case 6: // Bit
			tmp_entry_val = PyBool_FromLong(CharArrayWrapper_get_bit(ge_caw)); break;
		}
		if (tmp_entry_val == NULL) {
			goto error2;
		}
		PyTuple_SET_ITEM(event_tup, i, tmp_entry_val);
	}
	CharArrayWrapper_destroy(ge_caw);
	return event_tup;

error2: Py_DECREF(event_tup);
error1: CharArrayWrapper_destroy(ge_caw);
error0:
	return NULL;
}
#endif

// === GameEventDefinition

GameEventDefinition *GameEventDefinition_new() {
	GameEventDefinition *self = (GameEventDefinition *)malloc(sizeof(GameEventDefinition));
	if (self == NULL) {
		return NULL;
	}
	GameEventDefinition_init(self);
	return self;
}

void GameEventDefinition_init(GameEventDefinition *self) {
	self->event_type = 1 << 15;
	self->name = NULL;
	self->entries = NULL;
	self->entries_capacity = 0;
	self->entries_len = 0;
#ifndef NO_PYTHON
	self->py_name = NULL;
#endif
}

void GameEventDefinition_free(GameEventDefinition *self) {
	if (self->entries != NULL) {
		for (uint16_t i = 0; i < self->entries_len; i++) {
			GameEventEntry_free(self->entries + i);
		}
		free(self->entries);
		self->entries = NULL;
		self->entries_capacity = 0;
		self->entries_len = 0;
	}
	free(self->name);
#ifndef NO_PYTHON
	Py_XDECREF(self->py_name);
#endif
}

void GameEventDefinition_destroy(GameEventDefinition *self) {
	GameEventDefinition_free(self);
	free(self);
}

uint8_t GameEventDefinition_append_game_event_entry(GameEventDefinition *self, uint8_t *name, uint8_t type) {
	size_t previous_capacity = self->entries_capacity;
	uint8_t check_res = _generic_arraylist_size_check(
		sizeof(GameEventEntry), &self->entries, &self->entries_capacity, &self->entries_len
	);
	if (check_res >= 2) {
		return 1;
	}
	if (check_res == 1) {
		for (size_t i = self->entries_len; i < self->entries_capacity; i++) {
			GameEventEntry_init(self->entries + i);
		}
	}
	self->entries[self->entries_len].name = name;
	self->entries[self->entries_len].type = type;
	self->entries_len += 1;
	return 0;
}

#ifndef NO_PYTHON
PyObject *GameEventDefinition_get_field_names(GameEventDefinition *self) {
	PyObject *tup = PyTuple_New(self->entries_len);
	PyObject *name;
	if (tup == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < self->entries_len; i++) {
		name = GameEventEntry_get_python_name(self->entries + i);
		if (name == NULL) {
			Py_DECREF(tup);
			return NULL;
		}
		PyTuple_SET_ITEM(tup, i, self->entries[i].py_name);
	}
	return tup;
}

PyObject *GameEventDefinition_get_python_name(GameEventDefinition *self) {
	if (self->py_name == NULL) {
		self->py_name = PyUnicode_FromString(self->name);
		if (self->py_name == NULL) {
			return NULL;
		}
	}
	Py_INCREF(self->py_name);
	return self->py_name;
}
#endif
