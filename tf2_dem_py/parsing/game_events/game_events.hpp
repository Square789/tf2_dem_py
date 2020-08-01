#ifndef GAME_EVENTS__HPP
#define GAME_EVENTS__HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdint.h>

#include <vector>

namespace GameEvents {

// [length|ptr *]
//          '-> [event_type_id|event_type|name *|
//               entries_capacity|entries_length|entries *] ...
//                              [name *|type] ... <-'
// Where ... means the memory may repeat.
//

struct GameEventEntry {
	PyObject *name;
	uint8_t type;
	GameEventEntry(PyObject *name, uint8_t type):
		name(name), type(type) {};
};

/* Contains items to form a demo-specific definition of a game
 * event. */
struct GameEventDefinition {
	uint16_t event_type;
	PyObject *name;
	std::vector<GameEventEntry> entries;
};

}

#endif
