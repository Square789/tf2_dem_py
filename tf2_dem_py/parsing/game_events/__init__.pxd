
from libc.stdint cimport uint8_t, uint16_t

cdef extern from "tf2_dem_py/parsing/game_events/game_events.h":
	cdef struct GameEventEntry:
		char *name
		uint8_t type_ "type" # Rename on python layer

	cdef struct GameEventDefinition:
		uint16_t event_type_id
		uint16_t event_type
		char *name
		uint16_t entries_capacity
		uint16_t entries_length
		GameEventEntry *entries

	cdef struct GameEventDefinitionArray:
		uint16_t length
		GameEventDefinition *ptr

	void free_GameEventDefinitionArray(GameEventDefinitionArray *arrptr)
