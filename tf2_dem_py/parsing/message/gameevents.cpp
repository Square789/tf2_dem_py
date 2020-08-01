
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "tf2_dem_py/helpers.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/game_events/game_events.hpp"
#include "tf2_dem_py/parsing/message/__init__.hpp"
#include "tf2_dem_py/parsing/message/gameevents.hpp"

using ParserState::ParserState_c;

namespace MessageParsers {

static const uint8_t ENTRIES_SIZE_BLOCK = 4;

void read_game_event_definition(CharArrayWrapper *caw, ParserState_c *parser_state,
	GameEvents::GameEventDefinition *ged) {
	uint8_t last_type = 0;
	PyObject *entry_name;

	caw->read_raw(&(ged->event_type), 1, 1);
	ged->name = PyUnicode_FromCAWNulltrm(caw);
	if (ged->name == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	caw->read_raw(&last_type, 0, 3);
	while (last_type != 0) {
		entry_name = PyUnicode_FromCAWNulltrm(caw);
		if (entry_name == NULL) {
			parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
			return;
		}
		try {
			ged->entries.emplace_back(entry_name, last_type);
		} catch (std::bad_alloc& ba) {
			parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
			return;
		}
		caw->read_raw(&last_type, 0, 3);
	}
}

uint8_t inline should_read_game_event(uint16_t event_id) {
	switch (event_id) {
		case 23: return 1;
		default: return 0;
	}
}

void GameEvent::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	if (parser_state->game_event_defs == NULL) {
		this->skip(caw, parser_state); // Somehow received gameevent before gameevent defs
	}
	uint16_t length;
	caw->read_raw(&length, 1, 3);
	CharArrayWrapper *ge_caw = caw->caw_from_caw_b((uint64_t)length);
	uint16_t event_type = 0;
	GameEvents::GameEventDefinition *event_def = NULL;
	PyObject *tmp;
	PyObject *ge_dict = NULL;
	PyObject *entry_dict = NULL;
	PyObject *root_dict_ge_list;
	PyObject *tmp_entry_val = NULL;

	if (ge_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		goto error0;
	}

	ge_caw->read_raw(&event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto error1;
	}

	// Find event definition
	for (uint16_t def_idx = 0; def_idx < parser_state->game_event_def_amount; def_idx++) {
		if ((parser_state->game_event_defs[def_idx]).event_type == event_type) {
			event_def = &(parser_state->game_event_defs[def_idx]);
			break;
		}
	}

	if (event_def == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::UNKNOWN_GAME_EVENT;
		goto error1;
	}

	ge_dict = PyDict_New();
	if (ge_dict == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		goto error1;
	}
	entry_dict = PyDict_New();
	if (entry_dict == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		goto error2;
	}
	if (PyDict_SetItemString(ge_dict, "type", event_def->name) < 0) {
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		goto error3;
	}
	tmp = PyLong_FromLong(event_def->event_type);
	if (tmp == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		goto error3;
	}
	if (PyDict_SetItemString(ge_dict, "id", tmp) < 0) {
		Py_DECREF(tmp);
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		goto error3;
	}
	Py_DECREF(tmp);

	for (uint16_t i = 0; i < event_def->entries.size(); i++) {
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
			continue;
		}
		if (tmp_entry_val == NULL) {
			parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
			goto error3;
		}
		if (PyDict_SetItem(entry_dict, event_def->entries[i].name, tmp_entry_val) < 0) {
			parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
			Py_DECREF(tmp_entry_val);
			goto error3;
		}
		Py_DECREF(tmp_entry_val);
	}
	if (PyDict_SetItemString(ge_dict, "fields", entry_dict) < 0) {
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		goto error3;
	}
	root_dict_ge_list = PyDict_GetItemString(root_dict, "game_events");
	if (root_dict_ge_list == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		parser_state->FAILURE |= ParserState::ERRORS::UNKNOWN;
		goto error3;
	}
	if (PyList_Append(root_dict_ge_list, ge_dict) < 0) {
		parser_state->FAILURE |= ParserState::ERRORS::PYLIST;
	}
error3: Py_DECREF(entry_dict);
error2: Py_DECREF(ge_dict);
error1: delete ge_caw;
error0: return;
}

void GameEvent::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	uint16_t len = 0;
	caw->read_raw(&len, 1, 3);
	caw->skip(len / 8, len % 8);
}


void GameEventList::parse(CharArrayWrapper *caw, ParserState_c *parser_state, PyObject *root_dict) {
	uint16_t amount = 0;
	uint32_t length = 0;
	caw->read_raw(&amount, 1, 1);
	caw->read_raw(&length, 2, 4);
	if (caw->ERRORLEVEL != 0) {
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		return;
	}
	try {
		parser_state->game_event_defs = new GameEvents::GameEventDefinition[amount];
		parser_state->game_event_def_amount = amount;
	} catch(const std::bad_alloc& e) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	printf("Preparing to parse GameEventList: %u event defs", amount);

	CharArrayWrapper *gel_caw = caw->caw_from_caw_b(length);
	if (gel_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}
	for (uint16_t i = 0; i < amount; i++) {
		read_game_event_definition(gel_caw, parser_state, parser_state->game_event_defs + i);
		if (parser_state->FAILURE != 0) {
			goto error0;
		}
	}

error0:
	delete gel_caw;
}

void GameEventList::skip(CharArrayWrapper *caw, ParserState_c *parser_state) {
	caw->skip(1, 1);
	uint32_t len = 0;
	caw->read_raw(&len, 2, 4);
	caw->skip(len / 8, len % 8);
}

}
