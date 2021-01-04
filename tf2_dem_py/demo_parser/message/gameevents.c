#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/message/__init__.h"
#include "tf2_dem_py/demo_parser/message/gameevents.h"

uint8_t inline should_read_game_event(uint16_t event_id) {
	switch (event_id) {
		case 23: return 1;
		default: return 0;
	}
}

/* Read a game event definition into the parser state's game event definitions.
 * Will modify the parser state's FAILURE attribute on any error. */
void read_game_event_definition(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t last_type = 0;
	uint16_t event_id;
	uint8_t *entry_name;
	GameEventDefinition *ged;

	CharArrayWrapper_read_raw(caw, &event_id, 1, 1);
	if (event_id >= parser_state->game_event_def_amount) {
		parser_state->failure |= ParserState_ERR_GAME_EVENT_INDEX_OUTBOUND;
		return;
	}

	ged = parser_state->game_event_defs + event_id;

	ged->event_type = event_id;
	ged->name = CharArrayWrapper_get_nulltrm_str(caw);
	if (ged->name == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return;
	}

	CharArrayWrapper_read_raw(caw, &last_type, 0, 3);
	while (last_type != 0) {
		entry_name = CharArrayWrapper_get_nulltrm_str(caw);
		if (entry_name == NULL) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			return;
		}
		if (GameEventDefinition_append_game_event_entry(ged, entry_name, last_type) != 0) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			return;
		}
		CharArrayWrapper_read_raw(caw, &last_type, 0, 3);
	}

	// Create compact shell for game event
	// Not used anymore since python is removed from parsing //
// 	if ((parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) && should_read_game_event(event_id)) {
// 		PyObject *fieldnames_tuple, *comptup, *game_event_id;
// 		fieldnames_tuple = PyTuple_New(ged->entries_len);
// 		if (fieldnames_tuple == NULL) { goto error0; }
// 		comptup = CompactTuple3_Create();
// 		if (comptup == NULL) { goto error1; }

// 		// Fill field_names tuple
// 		for (uint16_t i = 0; i < ged->entries_len; i++) {
// 			Py_INCREF(ged->entries[i].name);
// 			PyTuple_SET_ITEM(fieldnames_tuple, i, ged->entries[i].name);
// 		}

// 		// Add game event name
// 		Py_INCREF(ged->name);
// 		PyTuple_SET_ITEM(comptup, CONSTANTS_COMPACT_TUPLE3_NAME_IDX, ged->name);
// 		// Add tuple
// 		PyTuple_SET_ITEM(comptup, CONSTANTS_COMPACT_TUPLE3_FIELD_NAMES_IDX, fieldnames_tuple);
// 		// Add comptup to container
// 		game_event_id = PyLong_FromLong(ged->event_type);
// 		if (game_event_id == NULL) { goto error2; }
// 		if (PyDict_SetItem(parser_state->game_event_container, game_event_id, comptup) < 0) {
// 			Py_DECREF(game_event_id);
// 			goto error2;
// 		}
// 		Py_DECREF(game_event_id); Py_DECREF(comptup);

// 		return;

// error2: Py_DECREF(comptup); goto error0; // Skip fieldnames_tuple decref, will be done by comptup
// error1: Py_DECREF(fieldnames_tuple); // Tuple dealloc decrefs everything inside
// error0:
// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
// 	}
}

void GameEvent_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	if (parser_state->game_event_defs == NULL) {
		GameEvent_skip(caw, parser_state); // Somehow received gameevent before gameevent defs
	}
	uint16_t length;
	CharArrayWrapper_read_raw(caw, &length, 1, 3);
	CharArrayWrapper *ge_caw = CharArrayWrapper_from_caw_b(caw, (uint64_t)length);
	uint16_t event_type = 0;
	GameEventDefinition *event_def;
	PyObject *tmp, *tmp1;
	PyObject *game_event_container;
	PyObject *gameevent_holder = NULL;
	PyObject *entry_dict = NULL;
	PyObject *tmp_entry_val = NULL;

	if (ge_caw == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error0;
	}

	CharArrayWrapper_read_raw(ge_caw, &event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto ret;
	}

	if (event_type > parser_state->game_event_def_amount) {
		parser_state->failure |= ParserState_ERR_GAME_EVENT_INDEX_OUTBOUND;
		goto error1;
	}

	event_def = parser_state->game_event_defs + event_type; // Pointer addition

	if (event_def->name == NULL) {
		parser_state->failure |= ParserState_ERR_UNKNOWN_GAME_EVENT;
		goto error1;
	}

	// Get either root.game_events or root.game_events.${eventId}.data into game_event_container
	// if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
	// 	tmp = PyLong_FromLong(event_def->event_type);
	// 	if (tmp == NULL) { parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION; goto error1; }
	// 	tmp1 = PyDict_GetItem(parser_state->game_event_container, tmp);
	// 	Py_DECREF(tmp);
	// 	if (tmp1 == NULL) { parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION; goto error1; }
	// 	game_event_container = PyTuple_GetItem(tmp1, CONSTANTS_COMPACT_TUPLE3_DATA_IDX);
	// } else {
	// 	game_event_container = parser_state->game_event_container;
	// }
	// if (game_event_container == NULL) {
	// 	parser_state->failure |= ParserState_ERR_UNKNOWN;
	// 	goto error1;
	// }

	// // Create either tuple (compact) or dict to hold this event's data
	// if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
	// 	gameevent_holder = PyTuple_New(event_def->entries_len);
	// } else {
	// 	gameevent_holder = PyDict_New();
	// }
	// if (gameevent_holder == NULL) {
	// 	parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 	goto error1;
	// }

	// // Create no fields dict when ge compact mode is enabled
	// if (!(parser_state->flags & FLAGS_COMPACT_GAME_EVENTS)) {
	// 	entry_dict = PyDict_New();
	// 	if (entry_dict == NULL) {
	// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 		goto error2;
	// 	}

	// 	// Set id field
	// 	tmp = PyLong_FromLong(event_def->event_type);
	// 	if (tmp == NULL) {
	// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 		goto error3;
	// 	}
	// 	if (PyDict_SetItem(gameevent_holder, CONSTANTS_DICT_NAMES_GameEvent->py_strings[0], tmp) < 0) {
	// 		Py_DECREF(tmp);
	// 		parser_state->failure |= ParserState_ERR_PYDICT;
	// 		goto error3;
	// 	}
	// 	Py_DECREF(tmp);

	// 	// Set name field
	// 	if (PyDict_SetItem(gameevent_holder, CONSTANTS_DICT_NAMES_GameEvent->py_strings[1], event_def->name) < 0) {
	// 		parser_state->failure |= ParserState_ERR_PYDICT;
	// 		goto error3;
	// 	}
	// }

	size_t orig_bytepos, new_bytepos;
	uint8_t orig_bitpos, new_bitpos;
	orig_bytepos = CharArrayWrapper_get_pos_byte(ge_caw);
	orig_bitpos = CharArrayWrapper_get_pos_bit(ge_caw);

	// Find out length of GameEvent data
	for (uint16_t i = 0; i < event_def->entries_len; i++) {
		switch (event_def->entries[i].type) {
		case 0:
			break;
		case 1: // String
			CharArrayWrapper_skip(ge_caw, CharArrayWrapper_dist_until_null(ge_caw), 0); break;
		case 2: // Float
			CharArrayWrapper_skip(ge_caw, 4, 0); break;
		case 3: // 32bInteger
			CharArrayWrapper_skip(ge_caw, 4, 0); break;
		case 4: // 16bInteger
			CharArrayWrapper_skip(ge_caw, 2, 0); break;
		case 5: // 8buInteger
			CharArrayWrapper_skip(ge_caw, 1, 0); break;
		case 6: // Bit
			CharArrayWrapper_skip(ge_caw, 0, 1); break;
		case 7: // "Local" / no idea what to do for this one
			break;
		}
	}

	new_bytepos = CharArrayWrapper_get_pos_byte(ge_caw);
	new_bitpos = CharArrayWrapper_get_pos_bit(ge_caw);

	size_t event_data_len = new_bytepos - orig_bytepos;
	if (new_bitpos > orig_bitpos) {
		event_data_len += 1;
	}

	// Rewind
	CharArrayWrapper_set_pos(ge_caw, orig_bytepos, orig_bitpos);

	// Allocate memory to hold GameEvent data
	uint8_t *event_data = (uint8_t *)malloc(event_data_len);
	if (event_data == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error1;
	}

	// Read it in
	CharArrayWrapper_read_raw(
		ge_caw,
		event_data,
		new_bytepos - orig_bytepos,
		(new_bitpos >= orig_bitpos) ? new_bitpos - orig_bitpos : (8 - (orig_bitpos - new_bitpos))
	);

	GameEvent *game_event = GameEvent_new();
	if (game_event == NULL) {
		free(event_data);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error1;
	}

	game_event->data = event_data;
	game_event->event_type = event_type;

	if (ParserState_append_game_event(parser_state, game_event) != 0) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error2;
	}

	// // Fill game_event dict
	// for (uint16_t i = 0; i < event_def->entries_len; i++) {
	// 	switch (event_def->entries[i].type) {
	// 	case 0:
	// 		break;
	// 	case 1: // String
	// 		tmp_entry_val = PyUnicode_FromCAWNulltrm(ge_caw); break;
	// 	case 2: // Float
	// 		tmp_entry_val = PyFloat_FromDouble(CharArrayWrapper_get_flt(ge_caw)); break;
	// 	case 3: // 32bInteger
	// 		tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int32(ge_caw)); break;
	// 	case 4: // 16bInteger
	// 		tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_int16(ge_caw)); break;
	// 	case 5: // 8buInteger
	// 		tmp_entry_val = PyLong_FromLong(CharArrayWrapper_get_uint8(ge_caw)); break;
	// 	case 6: // Bit
	// 		tmp_entry_val = PyBool_FromLong(CharArrayWrapper_get_bit(ge_caw)); break;
	// 	case 7: // "Local" ?????
	// 		// No clue
	// 		continue;
	// 	}
	// 	if (tmp_entry_val == NULL) {
	// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 		goto error3;
	// 	}
	// 	if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
	// 		Py_INCREF(tmp_entry_val);
	// 		PyTuple_SET_ITEM(gameevent_holder, i, tmp_entry_val);
	// 	} else {
	// 		if (PyDict_SetItem(entry_dict, event_def->entries[i].name, tmp_entry_val) < 0) {
	// 			parser_state->failure |= ParserState_ERR_PYDICT;
	// 			Py_DECREF(tmp_entry_val);
	// 			goto error3;
	// 		}
	// 	}
	// 	Py_DECREF(tmp_entry_val);
	// }

	// // Set entries dict
	// if (!(parser_state->flags & FLAGS_COMPACT_GAME_EVENTS)) {
	// 	if (PyDict_SetItem(gameevent_holder, CONSTANTS_DICT_NAMES_GameEvent->py_strings[2], entry_dict) < 0) {
	// 		parser_state->failure |= ParserState_ERR_PYDICT;
	// 		goto error3;
	// 	}
	// }
	// // Attach game event holder to container
	// if (PyList_Append(game_event_container, gameevent_holder) < 0) {
	// 	parser_state->failure |= ParserState_ERR_PYLIST;
	// 	goto error2; // As entry_dict is then decrefed by gameevent_holder
	// }

	CharArrayWrapper_destroy(ge_caw);

ret:
	return;

error2: GameEvent_destroy(game_event);
error1: CharArrayWrapper_destroy(ge_caw);
error0:
	return;
}

void GameEvent_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CharArrayWrapper_read_raw(caw, &len, 1, 3);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}


void GameEventList_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t amount = 0;
	uint32_t length = 0;
	CharArrayWrapper_read_raw(caw, &amount, 1, 1);
	CharArrayWrapper_read_raw(caw, &length, 2, 4);
	if (caw->ERRORLEVEL != 0) {
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
		goto error0;
	}
	if (parser_state->game_event_defs != NULL) { // This has like no reason or chance of happening but hey
		ParserState_free_game_event_defs(parser_state);
	}

	parser_state->game_event_defs = malloc(sizeof(GameEventDefinition) * amount);
	if (parser_state->game_event_defs == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error0;
	}
	// Proper initialization is done so GameEvent_parse can tell for sure if an undefined event has been received.
	for (uint16_t i = 0; i < amount; i++) {
		GameEventDefinition_init(parser_state->game_event_defs + i);
	}
	parser_state->game_event_def_amount = amount;

	//printf("Preparing to parse GameEventList: %u event defs\n", amount);
	//printf("maincaw:%uB%ub, ca_len:%u\n", caw->get_pos_byte(), caw->get_pos_bit(), caw->mem_len);

	CharArrayWrapper *gel_caw = CharArrayWrapper_from_caw_b(caw, length);
	if (gel_caw == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error0;
	}
	for (uint16_t _ = 0; _ < amount; _++) {
		read_game_event_definition(gel_caw, parser_state);
		if (parser_state->failure != 0) {
			goto error1;
		}
	}

error1:
	CharArrayWrapper_destroy(gel_caw);
error0:
	return;
}

void GameEventList_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 1);
	uint32_t len = 0;
	CharArrayWrapper_read_raw(caw, &len, 2, 4);
	CharArrayWrapper_skip(caw, len / 8, len % 8);
}
