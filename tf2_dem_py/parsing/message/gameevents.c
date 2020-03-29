
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/gameevents.h"

static const uint8_t ENTRIES_SIZE_BLOCK = 4;

// inline uint8_t read_game_event_entry(CharArrayWrapper *caw, GameEventEntry *target_ptr) {
// 	target_ptr->name = CAW_get_nulltrm_str(caw);
// 	CAW_read_raw(caw, &(target_ptr->value_type), 0, 3);
// 	printf("Read game event entry: %u, %s\n", target_ptr->value_type, target_ptr->name);
// 	return target_ptr->value_type;
// }

void read_game_event_definition(CharArrayWrapper *caw, ParserState *parser_state,
		GameEventDefinition *ged) {
	uint8_t last_type = 0;
	char *entry_name = NULL;
	GameEventEntry *tmp_new_entries = NULL;

	CAW_read_raw(caw, &(ged->event_type_id), 1, 1);
	ged->name = (char *)CAW_get_nulltrm_str(caw);
	ged->event_type = 512; // PLACEHOLDER
	ged->entries_capacity = ENTRIES_SIZE_BLOCK;
	ged->entries_length = 0;

	// Allocate block for GameEventEntries
	ged->entries = (GameEventEntry *)malloc(ENTRIES_SIZE_BLOCK * sizeof(GameEventEntry));
	if (ged->entries == NULL) {
		parser_state->FAILURE |= ERR_MEMORY_ALLOCATION;
		return;
	}
	memset(ged->entries, 0, ENTRIES_SIZE_BLOCK); // Zero it

	CAW_read_raw(caw, &last_type, 0, 3);
	while (last_type != 0) {
		entry_name = (char *)CAW_get_nulltrm_str(caw);
		(ged->entries + ged->entries_length)->type = last_type;
		(ged->entries + ged->entries_length)->name = entry_name;
		//printf(" Set type and name to %u, %s\n", last_type, entry_name);

		(ged->entries_length)++;

		// Extend capacity if exceeded
		if (ged->entries_length == ged->entries_capacity) {
			tmp_new_entries = (GameEventEntry *)realloc(
				ged->entries,
				sizeof(GameEventEntry) * (ENTRIES_SIZE_BLOCK + ged->entries_capacity)
			);
			if (tmp_new_entries == NULL) { // Failure check
				free(ged->entries);
				parser_state->FAILURE |= ERR_MEMORY_ALLOCATION;
				return;
			}
			ged->entries = tmp_new_entries;
			ged->entries_capacity += ENTRIES_SIZE_BLOCK;
			memset(
				ged->entries + ged->entries_capacity - ENTRIES_SIZE_BLOCK,
				0, ENTRIES_SIZE_BLOCK
			); // Zero out new array part
		}
		CAW_read_raw(caw, &last_type, 0, 3);
	}
}


void p_GameEvent(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	s_GameEvent(caw, parser_state);
}

void s_GameEvent(CharArrayWrapper *caw, ParserState *parser_state) {
	uint16_t len = 0;
	CAW_read_raw(caw, &len, 1, 3);
	CAW_skip(caw, len / 8, len % 8);
}


void p_GameEventList(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json) {
	uint16_t amount;
	uint32_t length;
	CAW_read_raw(caw, &amount, 1, 1);
	CAW_read_raw(caw, &length, 2, 4);
	GameEventDefinition *game_event_defs = malloc(amount * sizeof(GameEventDefinition));
	if (game_event_defs == NULL) {
		parser_state->FAILURE |= ERR_MEMORY_ALLOCATION;
		return;
	}
	printf("Reading %u GameEventDefs\n", amount);
	for (uint16_t i = 0; i < amount; i++) {
		read_game_event_definition(caw, parser_state, game_event_defs + i);
		if (parser_state->FAILURE != 0) {
			return;
		}
	}
}

void s_GameEventList(CharArrayWrapper *caw, ParserState *parser_state) {
	CAW_skip(caw, 1, 1);
	uint32_t len = 0;
	CAW_read_raw(caw, &len, 2, 4);
	//len >>= 8; // Shift unneccessary, why-ever.
	CAW_skip(caw, len / 8, len % 8);
}
