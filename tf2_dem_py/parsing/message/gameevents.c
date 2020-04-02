
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/game_events/game_events.h"
#include "tf2_dem_py/parsing/message/gameevents.h"

static const uint8_t ENTRIES_SIZE_BLOCK = 4;

void read_game_event_definition(CharArrayWrapper *caw, ParserState *parser_state,
		GameEventDefinition *ged) {
	uint8_t last_type = 0;
	char *entry_name = NULL;
	GameEventEntry *tmp_new_entries = NULL;

	CAW_read_raw(caw, &(ged->event_type), 1, 1);
	ged->name = (char *)CAW_get_nulltrm_str(caw);
	ged->entries_capacity = ENTRIES_SIZE_BLOCK;
	ged->entries_length = 0;

	// Allocate block for GameEventEntries
	ged->entries = (GameEventEntry *)malloc(ENTRIES_SIZE_BLOCK * sizeof(GameEventEntry));
	if (ged->entries == NULL) {
		parser_state->FAILURE |= ERR.MEMORY_ALLOCATION;
		return;
	}
	memset(ged->entries, 0, sizeof(GameEventEntry) * ENTRIES_SIZE_BLOCK); // Zero it

	CAW_read_raw(caw, &last_type, 0, 3);
	while (last_type != 0) {
		entry_name = (char *)CAW_get_nulltrm_str(caw);
		(ged->entries + ged->entries_length)->type = last_type;
		(ged->entries + ged->entries_length)->name = entry_name;
		(ged->entries_length) += 1; // Read entry's two attributes and bump length
		CAW_read_raw(caw, &last_type, 0, 3);

		// Extend capacity if exceeded
		if (ged->entries_length == ged->entries_capacity) {
			tmp_new_entries = (GameEventEntry *)realloc(
				ged->entries,
				sizeof(GameEventEntry) * (ENTRIES_SIZE_BLOCK + ged->entries_capacity)
			);
			if (tmp_new_entries == NULL) { // Failure check
				parser_state->FAILURE |= ERR.MEMORY_ALLOCATION;
				return;
			}
			ged->entries = tmp_new_entries;
			ged->entries_capacity += ENTRIES_SIZE_BLOCK;
			memset(
				ged->entries + ged->entries_capacity - ENTRIES_SIZE_BLOCK,
				0, ENTRIES_SIZE_BLOCK
			); // Zero out new array part
		}
	}
}

uint8_t inline should_read_game_event(event_id) {
	switch (event_id) {
	case 23:
		return 1;
	default:
		return 0;
	}
}


void p_GameEvent(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	if (parser_state->game_event_defs == NULL) {
		s_GameEvent(caw, parser_state); // Somehow received gameevent before gameevent defs
	}
	uint16_t length;
	CAW_read_raw(caw, &length, 1, 3);
	CharArrayWrapper *ge_caw = CAW_from_caw_b(caw, (uint64_t)length);
	if (ge_caw == NULL) {
		parser_state->FAILURE |= ERR.MEMORY_ALLOCATION;
		return;
	}
	uint16_t event_type = 0;
	GameEventDefinition *event_def = NULL;

	CAW_read_raw(ge_caw, &event_type, 1, 1);

	if (!should_read_game_event(event_type)) {
		goto cleanup_and_ret;
	}

	for (uint16_t def_idx = 0; def_idx < parser_state->game_event_defs->length; def_idx++) {
		if ((parser_state->game_event_defs->ptr[def_idx]).event_type == event_type) {
			event_def = &(parser_state->game_event_defs->ptr[def_idx]);
			break;
		}
	} // Find event definition
	cJSON *ge_json = cJSON_CreateObject();
	cJSON *entry_json = cJSON_CreateObject();
	cJSON_AddStringToObject(ge_json, "type", event_def->name);
	cJSON_AddNumberToObject(ge_json, "id", event_def->event_type);
	if (event_def == NULL || ge_json == NULL || entry_json == NULL) {
		parser_state->FAILURE |= ERR.UNKNOWN_GAME_EVENT;
		goto cleanup_and_ret;
	}

	for (uint16_t i = 0; i < event_def->entries_length; i++) {
		switch (event_def->entries[i].type) {
		case 0:
			break;
		case 1: //String
			cJSON_AddVolatileStringRefToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_nulltrm_str(ge_caw));
			break;
		case 2:
			cJSON_AddNumberToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_flt(ge_caw));
			break;
		case 3:
			cJSON_AddNumberToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_uint32(ge_caw));
			break;
		case 4:
			cJSON_AddNumberToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_uint16(ge_caw));
			break;
		case 5:
			cJSON_AddNumberToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_uint8(ge_caw));
			break;
		case 6:
			cJSON_AddNumberToObject(
				entry_json, event_def->entries[i].name,
				CAW_get_bit(ge_caw));
			break;
		case 7:
			// No clue
			break;
		}
	}
	cJSON_AddItemToObject(ge_json, "fields", entry_json);
	cJSON_AddItemToArray(
		cJSON_GetObjectItemCaseSensitive(root_json, "game_events"),
		ge_json
	);
cleanup_and_ret:
	CAW_delete(ge_caw);
	return;
}

void s_GameEvent(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}


void p_GameEventList(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint16_t amount = 0;
	uint32_t length = 0;
	CAW_read_raw(caw, &amount, 1, 1);
	CAW_read_raw(caw, &length, 2, 4);
	GameEventDefinitionArray *ged_arr = malloc(sizeof(GameEventDefinitionArray));
	GameEventDefinition *game_event_defs = malloc(amount * sizeof(GameEventDefinition));
	CharArrayWrapper *gel_caw = CAW_from_caw_b(caw, length);
	if (game_event_defs == NULL || ged_arr == NULL || gel_caw == NULL) {
		parser_state->FAILURE |= ERR.MEMORY_ALLOCATION;
		return;
	}

	printf("Reading %u GameEventDefs\n", amount);
	for (uint16_t i = 0; i < amount; i++) {
		read_game_event_definition(gel_caw, parser_state, game_event_defs + i);
		if (parser_state->FAILURE != 0) {
			return;
		}
	}
	ged_arr->length = amount;
	ged_arr->ptr = game_event_defs;

	parser_state->game_event_defs = ged_arr;
	CAW_delete(gel_caw);
}

void s_GameEventList(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 1);
	uint32_t len = 0;
	CAW_read_raw(caw, &len, 2, 4);
	CAW_skip(caw, len / 8, len % 8);
}
