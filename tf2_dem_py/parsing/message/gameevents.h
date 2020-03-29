#ifndef MESSAGE_GAMEEVENTS__H
#define MESSAGE_GAMEEVENTS__H

#include "tf2_dem_py/cJSON/cJSON.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

typedef struct GameEventEntry {
	char *name;
	uint8_t type;
} GameEventEntry;

/* Contains items to form a demo-specific definition of a game
 * event. */
typedef struct GameEventDefinition {
	uint16_t event_type_id;
	uint16_t event_type;
	char *name;
	uint16_t entries_capacity;
	uint16_t entries_length;
	GameEventEntry *entries;
} GameEventDefinition;

void p_GameEvent(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_GameEvent(CharArrayWrapper *caw, ParserState *parser_state);

void p_GameEventList(CharArrayWrapper *caw, ParserState *parser_state, cJSON *root_json);
void s_GameEventList(CharArrayWrapper *caw, ParserState *parser_state);

#endif
