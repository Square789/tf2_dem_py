#ifndef SERVER_INFO__H
#define SERVER_INFO__H

#include <stdint.h>
#include <stdbool.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"

#define PY_SSIZE_T_CLEAN
#include "Python.h"

typedef struct ServerInfo_s {
	uint16_t version;
	uint32_t server_count;
	bool     stv;
	bool     dedicated;
	uint32_t max_crc;
	uint16_t max_classes;
	uint8_t  map_hash[16];
	uint8_t  player_count;
	uint8_t  max_player_count;
	float    interval_per_tick;
	uint8_t  platform;
	uint8_t *game;
	uint8_t *map_name;
	uint8_t *skybox;
	uint8_t *server_name;
	bool     replay;
} ServerInfo;

ServerInfo *ServerInfo_new();
void ServerInfo_init(ServerInfo *self);
void ServerInfo_destroy(ServerInfo *self);

// Reads data from a CharArrayWrapper into a ServerInfo struct.
// On failure, the CharArrayWrapper's ERRORLEVEL will be set.
void ServerInfo_read(ServerInfo *self, CharArrayWrapper *caw);

// Converts a filled ServerInfo struct to a Python dict.
// If the passed in ServerInfo pointer is NULL, None is returned.
// CONSTANTS need to be initialized.
// Returns the dict or None on success, NULL on failure.
PyObject *ServerInfo_to_PyDict(ServerInfo *self);

#endif
