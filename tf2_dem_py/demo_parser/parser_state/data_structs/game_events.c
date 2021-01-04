#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/demo_parser/parser_state/data_structs/game_events.h"

GameEventEntry *GameEventEntry_new(uint8_t *name, uint8_t type) {
	GameEventEntry *self = (GameEventEntry *)malloc(sizeof(GameEventEntry));
	if (self == NULL) {
		return NULL;
	}
	self->name = name;
	self->type = type;
	return self;
}

void GameEventEntry_destroy(GameEventEntry *self) {
	free(self->name);
	// free(self); // Same as GameEventDefinition here, only laid out in a direct array, never
	// introduced to malloc & Co.
}


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
	self->entries_capacity = 0;
	self->entries_len = 0;
	self->entries = NULL;
}

void GameEventDefinition_destroy(GameEventDefinition *self) {
	if (self->entries != NULL) {
		for (uint16_t i = 0; i < self->entries_len; i++) {
			GameEventEntry_destroy(self->entries + i);
		}
	}
	free(self->name);
	// free(self); // This is really ugly and hackish since the only instance where GEDs are
	// created is actually just in a plain array, so they should not be freed.
	// Rework this, probably
}

uint8_t GameEventDefinition_append_game_event_entry(GameEventDefinition *self, uint8_t *name, uint8_t type) {
	MACRO_ARRAYLIST_APPEND(GameEventDefinition, self->entries, self->entries_capacity, self->entries_len)
	self->entries[self->entries_len].name = name;
	self->entries[self->entries_len].type = type;
	self->entries_len += 1;
	return 0;
}


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
	self->event_type = 0;
}

void GameEvent_destroy(GameEvent *self) {
	free(self->data);
	free(self);
}
