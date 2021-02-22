#ifndef PARSER_STATE_DEMO_HEADER__H
#define PARSER_STATE_DEMO_HEADER__H

#include <stdint.h>
#include <stdio.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"

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

// Read data into a DemoHeader from a file pointer, advancing it by 1072 bytes.
// Returns 1 on CAW memory allocation failure, 2 on general CAW error, 0 on success.
// Will write CAW error into the address pointed to by caw_err if return code is 2.
uint8_t DemoHeader_read(DemoHeader *self, FILE *stream, CharArrayWrapper_err_t *caw_err);

#endif
