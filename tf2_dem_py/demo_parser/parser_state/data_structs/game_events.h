#ifndef GAME_EVENTS__HPP
#define GAME_EVENTS__HPP

#include <stdint.h>

typedef struct {
	uint8_t *name;
	uint8_t type;
} GameEventEntry;

GameEventEntry *GameEventEntry_new(uint8_t *name, uint8_t type);
void GameEventEntry_delete(GameEventEntry *self);

// When parsing, a large array of these is created to hold all relevant GameEvents

// Contains items to form a demo-specific definition of a game event.
typedef struct {
	uint16_t event_type;
	uint8_t *name;
	uint16_t entries_capacity;
	uint16_t entries_len;
	GameEventEntry *entries;
} GameEventDefinition;

GameEventDefinition *GameEventDefinition_new();
void GameEventDefinition_init(GameEventDefinition *self);
void GameEventDefinition_destroy(GameEventDefinition *self);

// Append a new GameEventEntry to the array of entries, or create a new one if it's NULL.
// Reference count for `name` is incremented only on success.
// Will return 0 on success, 1 on any sort of allocation failure.
uint8_t GameEventDefinition_append_game_event_entry(GameEventDefinition *self, uint8_t *name, uint8_t type);

// Struct for holding extracted GameEvent data
typedef struct {
	uint16_t event_type;
	uint8_t *data;
} GameEvent;

GameEvent *GameEvent_new();
void GameEvent_init(GameEvent *self);
void GameEvent_destroy(GameEvent *self);

#endif
