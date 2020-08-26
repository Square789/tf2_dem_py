
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <vector>

#include "tf2_dem_py/parsing/helpers.hpp"
#include "tf2_dem_py/constants.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/flags/flags.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"

#include "tf2_dem_py/parsing/game_events/game_events.hpp"
#include "tf2_dem_py/parsing/message/__init__.hpp"
#include "tf2_dem_py/parsing/message/gameevents.hpp"

using ParserState::ParserState_c;

namespace MessageParsers {

void read_game_event_definition(CharArrayWrapper *caw, ParserState_c *parser_state) {
	uint8_t last_type = 0;
	uint16_t event_id;
	PyObject *entry_name;
	GameEvents::GameEventDefinition *ged_array = parser_state->game_event_defs;

	//printf("gel_caw:%uB%ub\n", caw->get_pos_byte(), caw->get_pos_bit());
	caw->read_raw(&event_id, 1, 1);
	//printf("read event ID %u\n", event_id);
	if (event_id >= parser_state->game_event_def_amount) {
		parser_state->FAILURE |= ParserState::ERRORS::GAME_EVENT_INDEX_OUTBOUND;
		return;
	}

	ged_array[event_id].event_type = event_id;
	ged_array[event_id].name = PyUnicode_FromCAWNulltrm(caw);
	if (ged_array[event_id].name == NULL) {
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
			ged_array[event_id].entries.emplace_back(entry_name, last_type);
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
	GameEvents::GameEventDefinition *event_def;
	PyObject *tmp;
	PyObject *ge_dict = NULL;
	PyObject *entry_dict = NULL;
	PyObject *root_ge_container;
	PyObject *tmp_entry_val = NULL;

	if (ge_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		goto error0;
	}

	ge_caw->read_raw(&event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto error1;
	}

	if (event_type > parser_state->game_event_def_amount) {
		parser_state->FAILURE |= ParserState::ERRORS::GAME_EVENT_INDEX_OUTBOUND; goto error1; }

	event_def = parser_state->game_event_defs + event_type; // Pointer addition

	if (event_def->name == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::UNKNOWN_GAME_EVENT; goto error1; }

	ge_dict = PyDict_New();
	if (ge_dict == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION; goto error1; }

	entry_dict = PyDict_New();
	if (entry_dict == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION; goto error2; }

	// Set id field
	tmp = PyLong_FromLong(event_def->event_type);
	if (tmp == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION; goto error3; }
	if (PyDict_SetItem(ge_dict, CONSTANTS::DICT_NAMES_GameEvent->py_strings[0], tmp) < 0) {
		Py_DECREF(tmp);
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		goto error3;
	}
	Py_DECREF(tmp);

	// Set type field
	if (PyDict_SetItem(ge_dict, CONSTANTS::DICT_NAMES_GameEvent->py_strings[1], event_def->name) < 0) {
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT; goto error3; }

	// Fill game_event dict
	for (uint16_t i = 0; i < event_def->entries.size(); i++) {
		switch (event_def->entries[i].type) {
		case 0:
			break;
		case 1: // String
			tmp_entry_val = PyUnicode_FromCAWNulltrm(ge_caw); break;
		case 2: // Float
			tmp_entry_val = PyFloat_FromDouble(ge_caw->get_flt()); break;
		case 3: // 32bInteger
			tmp_entry_val = PyLong_FromLong(ge_caw->get_int32()); break;
		case 4: // 16bInteger
			tmp_entry_val = PyLong_FromLong(ge_caw->get_int16()); break;
		case 5: // 8buInteger
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

	// Set game_event dict
	if (PyDict_SetItem(ge_dict, CONSTANTS::DICT_NAMES_GameEvent->py_strings[2], entry_dict) < 0) {
		parser_state->FAILURE |= ParserState::ERRORS::PYDICT;
		goto error3;
	}
	root_ge_container = parser_state->game_event_container;
	if (root_ge_container == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::UNKNOWN;
		goto error3;
	}
	if (PyList_Append(root_ge_container, ge_dict) < 0) {
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
	if (parser_state->game_event_defs != NULL) { // This has like no reason or chance of happening but hey
		delete[] parser_state->game_event_defs;
		parser_state->game_event_defs = NULL;
	}
	try {
		parser_state->game_event_defs = new GameEvents::GameEventDefinition[amount]; //CONST::MAX_GAME_EVENT_AMOUNT
		parser_state->game_event_def_amount = amount;
	} catch(const std::bad_alloc& e) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}

	printf("Preparing to parse GameEventList: %u event defs\n", amount);

	//printf("maincaw:%uB%ub, ca_len:%u\n", caw->get_pos_byte(), caw->get_pos_bit(), caw->mem_len);
	CharArrayWrapper *gel_caw = caw->caw_from_caw_b(length);
	if (gel_caw == NULL) {
		parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION;
		return;
	}
	for (uint16_t _ = 0; _ < amount; _++) {
		read_game_event_definition(gel_caw, parser_state);
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
