
#include <stdlib.h>

#include "tf2_dem_py/parsing/game_events/game_events.hpp"

void free_GameEventDefinitionArray(GameEventDefinitionArray *arrptr) {
	if (arrptr == NULL) {
		return;
	}
	for (uint16_t i = 0; i < arrptr->length; i++) {
		free((arrptr->ptr)[i].name); // Free name of GameEventDefinition
		for (uint16_t j = 0; i < (arrptr->ptr)[i].entries_length; i++) {
			// Free entries' names
			free(((arrptr->ptr)[i].entries)[j].name);
		}
		free((arrptr->ptr)[i].entries); // Free Entries for that event
	}
	free(arrptr->ptr); // Free block storing GameEventDefinitions
	free(arrptr); // Free GameEventDefinitionArray struct
}
