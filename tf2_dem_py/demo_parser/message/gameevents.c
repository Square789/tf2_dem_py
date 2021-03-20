#include <math.h>
#include <stdbool.h>
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

inline bool should_read_game_event(uint16_t event_id) {
	switch (event_id) {
		case 23:
			return true;
		default:
			return false;
	}
}

/* Read a game event definition into the parser state's game event definitions.
 * Will modify the parser state's FAILURE attribute on any error. */
void read_game_event_definition(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t last_type = 0;
	uint16_t event_id = 0;
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
}

void GameEvent_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	if (parser_state->game_event_defs == NULL) {
		GameEvent_skip(caw, parser_state); // Somehow received gameevent before gameevent defs
		return;
	}
	uint16_t length = 0;
	CharArrayWrapper_read_raw(caw, &length, 1, 3);
	CharArrayWrapper *ge_caw = CharArrayWrapper_from_caw_b(caw, length);
	uint16_t event_type = 0;
	GameEventDefinition *event_def;

	if (ge_caw == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error0;
	}

	CharArrayWrapper_read_raw(ge_caw, &event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto do_not_read;
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

	size_t event_data_len = new_bytepos - orig_bytepos + (new_bitpos > orig_bitpos);

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
		new_bytepos - orig_bytepos - (new_bitpos < orig_bitpos),
		(new_bitpos >= orig_bitpos) ? new_bitpos - orig_bitpos : (8 - (orig_bitpos - new_bitpos))
	);

	GameEvent game_event = {
		.data = event_data,
		.data_len = event_data_len,
		.event_type = event_type,
	};

	if (ParserState_append_game_event(parser_state, game_event) != 0) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		goto error1;
	}

do_not_read:
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

	// printf("Preparing to parse GameEventList: %u event defs\n", amount);
	// printf("maincaw:%uB%ub, ca_len:%u\n", CharArrayWrapper_get_pos_byte(caw), CharArrayWrapper_get_pos_bit(caw), caw->mem_len);

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
