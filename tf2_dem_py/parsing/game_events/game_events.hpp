#ifndef GAME_EVENTS__HPP
#define GAME_EVENTS__HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>

// [length|ptr *]
//          '-> [event_type_id|event_type|name *|
//               entries_capacity|entries_length|entries *] ...
//                              [name *|type] ... <-'
// Where ... means the memory may repeat.
//

typedef struct GameEventEntry {
	PyObject *name;
	uint8_t type;
} GameEventEntry;

/* Contains items to form a demo-specific definition of a game
 * event. */
typedef struct GameEventDefinition {
	uint16_t event_type;
	PyObject *name;
	uint16_t entries_capacity;
	uint16_t entries_length;
	GameEventEntry *entries;
} GameEventDefinition;

/* Contains length of GameEventDefinition array and a pointer to it. */
typedef struct GameEventDefinitionArray {
	uint16_t length;
	GameEventDefinition *ptr;
} GameEventDefinitionArray;

void free_GameEventDefinitionArray(GameEventDefinitionArray *ptr);

#endif
