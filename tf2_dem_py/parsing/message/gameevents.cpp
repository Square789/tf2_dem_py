
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/helpers.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/game_events/game_events.hpp"
#include "tf2_dem_py/parsing/message/__init__.hpp"
#include "tf2_dem_py/parsing/message/gameevents.hpp"

namespace MessageParsers {

static const uint8_t ENTRIES_SIZE_BLOCK = 4;

void read_game_event_definition(CharArrayWrapper *caw, ParserState *parser_state,
		GameEventDefinition *ged) {
	uint8_t last_type = 0;
	PyObject *entry_name;
	GameEventEntry *tmp_new_entries = NULL;

	caw->read_raw(&(ged->event_type), 1, 1);
	ged->name = PyUnicode_FromCAWNulltrm(caw);
	if (ged->name == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}
	ged->entries_capacity = ENTRIES_SIZE_BLOCK;
	ged->entries_length = 0;

	// Allocate block for GameEventEntries
	ged->entries = (GameEventEntry *)malloc(ENTRIES_SIZE_BLOCK * sizeof(GameEventEntry));
	if (ged->entries == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}

	caw->read_raw(&last_type, 0, 3);
	while (last_type != 0) {
		entry_name = PyUnicode_FromCAWNulltrm(caw);
		if (entry_name == NULL) {
			parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
			return;
		}
		(ged->entries + ged->entries_length)->type = last_type;
		(ged->entries + ged->entries_length)->name = entry_name;
		(ged->entries_length) += 1; // Read entry's two attributes and bump length
		caw->read_raw(&last_type, 0, 3);

		// Extend capacity if exceeded
		if (ged->entries_length == ged->entries_capacity) {
			tmp_new_entries = (GameEventEntry *)realloc(
				ged->entries,
				sizeof(GameEventEntry) * (ENTRIES_SIZE_BLOCK + ged->entries_capacity)
			);
			if (tmp_new_entries == NULL) { // Failure check
				parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
				return;
			}
			ged->entries = tmp_new_entries;
			ged->entries_capacity += ENTRIES_SIZE_BLOCK;
		}
	}
}

uint8_t inline should_read_game_event(uint16_t event_id) {
	switch (event_id) {
		case 23: return 1;
		default: return 0;
	}
}

void GameEvent::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	if (parser_state->game_event_defs == NULL) {
		this->skip(caw, parser_state); // Somehow received gameevent before gameevent defs
	}
	uint16_t length;
	caw->read_raw(&length, 1, 3);
	CharArrayWrapper *ge_caw = caw->caw_from_caw_b((uint64_t)length);
	uint16_t event_type = 0;
	GameEventDefinition *event_def = NULL;
	PyObject *ge_dict;
	PyObject *entry_dict;
	PyObject *root_dict_ge_list;
	PyObject *tmp_entry_val = NULL;

	if (ge_caw == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}

	ge_caw->read_raw(&event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto cleanup_and_ret;
	}

	for (uint16_t def_idx = 0; def_idx < parser_state->game_event_defs->length; def_idx++) {
		if ((parser_state->game_event_defs->ptr[def_idx]).event_type == event_type) {
			event_def = &(parser_state->game_event_defs->ptr[def_idx]);
			break;
		}
	} // Find event definition

	if (event_def == NULL) {
		parser_state->FAILURE |= ParserState_ERR.UNKNOWN_GAME_EVENT;
		goto cleanup_and_ret;
	}

	ge_dict = PyDict_New();
	entry_dict = PyDict_New();
	if (ge_dict == NULL || entry_dict == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		goto cleanup_and_ret;
	}
	if (PyDict_SetItemString(ge_dict, "type", event_def->name) < 0 ||
		PyDict_SetItemString(ge_dict, "id", PyLong_FromLong(event_def->event_type)) < 0) {
		parser_state->FAILURE |= ParserState_ERR.PYDICT;
		goto cleanup_and_ret;
	}

	for (uint16_t i = 0; i < event_def->entries_length; i++) {
		switch (event_def->entries[i].type) {
		case 0:
			break;
		case 1: // String
			tmp_entry_val = PyUnicode_FromCAWNulltrm(ge_caw); break;
		case 2: // Float
			tmp_entry_val = PyFloat_FromDouble(ge_caw->get_flt()); break;
		case 3: // 32bInteger
			tmp_entry_val = PyLong_FromLong(ge_caw->get_uint32()); break;
		case 4: // 16bInteger
			tmp_entry_val = PyLong_FromLong(ge_caw->get_uint16()); break;
		case 5: // 8bInteger
			tmp_entry_val = PyLong_FromLong(ge_caw->get_uint8()); break;
		case 6: // Bit
			tmp_entry_val = PyBool_FromLong(ge_caw->get_bit()); break;
		case 7: // "Local" ?????
			// No clue
			break;
		}
		if (event_def->entries[i].type == 7) {
			continue;
		}
		if (tmp_entry_val == NULL) {
			parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
			goto cleanup_and_ret;
		}
		PyDict_SetItem(entry_dict, event_def->entries[i].name, tmp_entry_val);
		Py_DECREF(tmp_entry_val);
	}
	if (PyDict_SetItemString(ge_dict, "fields", entry_dict) < 0) {
		parser_state->FAILURE |= ParserState_ERR.PYDICT;
	}
	root_dict_ge_list = PyDict_GetItemString(root_dict, "game_events");
	if (root_dict_ge_list == NULL) {
		parser_state->FAILURE |= ParserState_ERR.PYDICT;
		parser_state->FAILURE |= ParserState_ERR.UNKNOWN;
		goto cleanup_and_ret;
	}
	if (PyList_Append(root_dict_ge_list, ge_dict) < 0) {
		parser_state->FAILURE |= ParserState_ERR.PYLIST;
	}
cleanup_and_ret:
	delete ge_caw;
	return;
}

void GameEvent::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	caw->read_raw(&len, 1, 3);
	caw->skip(len / 8, len % 8);
}


void GameEventList::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	uint16_t amount = 0;
	uint32_t length = 0;
	caw->read_raw(&amount, 1, 1);
	caw->read_raw(&length, 2, 4);
	if (caw->ERRORLEVEL != 0) {
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		return;
	}
	GameEventDefinitionArray *ged_arr = (GameEventDefinitionArray *)malloc(sizeof(GameEventDefinitionArray));
	GameEventDefinition *game_event_defs = (GameEventDefinition *)malloc(amount * sizeof(GameEventDefinition));
	CharArrayWrapper *gel_caw = caw->caw_from_caw_b(length);
	if (game_event_defs == NULL || ged_arr == NULL || gel_caw == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}

	for (uint16_t i = 0; i < amount; i++) {
		read_game_event_definition(gel_caw, parser_state, game_event_defs + i);
		if (parser_state->FAILURE != 0) {
			goto func_end;
		}
	}
	ged_arr->length = amount;
	ged_arr->ptr = game_event_defs;

	parser_state->game_event_defs = ged_arr;
func_end:
	delete gel_caw;
}

void GameEventList::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 1);
	uint32_t len = 0;
	caw->read_raw(&len, 2, 4);
	caw->skip(len / 8, len % 8);
}

}
