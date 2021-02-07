#ifndef PARSER_STATE_DEMO_HEADER__H
#define PARSER_STATE_DEMO_HEADER__H

#include <stdint.h>
#include <stdio.h>

#define PY_SSIZE_T_CLEAN
#include "Python.h"

typedef struct {
	uint8_t *ident;
	uint32_t net_prot;
	uint32_t dem_prot;
	uint8_t *host_addr;
	uint8_t *client_id;
	uint8_t *map_name;
	uint8_t *game_dir;
	float play_time;
	uint32_t tick_count;
	uint32_t frame_count;
	uint32_t sigon;
} DemoHeader;

DemoHeader *DemoHeader_new();
void DemoHeader_init(DemoHeader *self);
void DemoHeader_destroy(DemoHeader *self);

// Create a Python dict from a Demo Header and return it.
// Only call this function if the DemoHeader contains valid data and no
// NULL pointers!
// Requires CONSTANTS to be initialized.
// Returns NULL on failure.
PyObject *DemoHeader_to_PyDict(DemoHeader *self);

#endif
