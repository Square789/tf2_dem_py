#ifndef TF2_DEM_PY_FLAGS__H
#define TF2_DEM_PY_FLAGS__H

#include <stdint.h>

typedef struct FLAGS_s {
	uint16_t CHAT;
	uint16_t GAME_EVENTS;
} FLAGS_s;

extern const FLAGS_s FLAGS;

#endif
