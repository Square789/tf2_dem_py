#ifndef GAME_EVENTS__H
#define GAME_EVENTS__H

#include <stdint.h>

#ifndef NO_PYTHON
#  define PY_SSIZE_T_CLEAN
#  include "Python.h"
#endif

// An entry in a GameEventDefinition, holding the entry's name and type
typedef struct GameEventEntry_s {
	uint8_t *name;
	uint8_t type;
} GameEventEntry;

// Contains items to form a demo-specific definition of a game event.
typedef struct GameEventDefinition_s {
	uint16_t event_type;
	uint8_t *name;
	size_t entries_capacity;
	size_t entries_len;
	GameEventEntry *entries;
} GameEventDefinition;

// Struct for holding extracted GameEvent data
typedef struct GameEvent_s {
	uint16_t event_type;
	uint8_t *data;
	size_t data_len;
} GameEvent;

GameEventEntry *GameEventEntry_new();
void GameEventEntry_init(GameEventEntry *self);
void GameEventEntry_free(GameEventEntry *self);
void GameEventEntry_destroy(GameEventEntry *self);

GameEventDefinition *GameEventDefinition_new();
void GameEventDefinition_init(GameEventDefinition *self);
void GameEventDefinition_free(GameEventDefinition *self);
void GameEventDefinition_destroy(GameEventDefinition *self);

GameEvent *GameEvent_new();
void GameEvent_init(GameEvent *self);
void GameEvent_free(GameEvent *self);
void GameEvent_destroy(GameEvent *self);

// Appends a new GameEventEntry to the array of entries; create a new one if it's NULL.
// Will return 0 on success, 1 on any sort of allocation failure.
uint8_t GameEventDefinition_append_game_event_entry(GameEventDefinition *self, uint8_t *name, uint8_t type);

#ifndef NO_PYTHON
// Returns a python tuple of python strings of the game event's field names.
PyObject *GameEventDefinition_get_field_names(GameEventDefinition *self);

// Converts a GameEvent to a Python dict.
// Returns NULL on any sort of failure, may or may not raise a python error.
PyObject *GameEvent_to_PyDict(GameEvent *self, GameEventDefinition *event_def);
PyObject *GameEvent_to_compact_PyTuple(GameEvent *self, GameEventDefinition *event_def);
#endif

#endif
