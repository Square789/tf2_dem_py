#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/demo_parser/data_structs/server_info.h"

ServerInfo *ServerInfo_new() {
	ServerInfo *self = (ServerInfo *)malloc(sizeof(ServerInfo));
	if (self == NULL) {
		return NULL;
	}
	ServerInfo_init(self);
	return self;
}

void ServerInfo_init(ServerInfo *self) {
	self->version = 0;
	self->server_count = 0;
	self->stv = 0;
	self->dedicated = 0;
	self->max_crc = 0;
	self->max_classes = 0;
	memset(self->map_hash, 0, 16);
	self->player_count = 0;
	self->max_player_count = 0;
	self->interval_per_tick = 0;
	self->platform = 0;
	self->game = NULL;
	self->map_name = NULL;
	self->skybox = NULL;
	self->server_name = NULL;
	self->replay = 0;
}

void ServerInfo_free(ServerInfo *self) {
	free(self->game);
	free(self->map_name);
	free(self->skybox);
	free(self->server_name);
}

void ServerInfo_destroy(ServerInfo *self) {
	ServerInfo_free(self);
	free(self);
}

void ServerInfo_read(ServerInfo *self, CharArrayWrapper *caw) {
	self->version =           CharArrayWrapper_get_uint16(caw);
	self->server_count =      CharArrayWrapper_get_uint32(caw);
	self->stv =               CharArrayWrapper_get_bit(caw);
	self->dedicated =         CharArrayWrapper_get_bit(caw);
	self->max_crc =           CharArrayWrapper_get_uint32(caw);
	self->max_classes =       CharArrayWrapper_get_uint16(caw);
	CharArrayWrapper_read_raw(caw, self->map_hash, 16, 0);
	self->player_count =      CharArrayWrapper_get_uint8(caw);
	self->max_player_count =  CharArrayWrapper_get_uint8(caw);
	self->interval_per_tick = CharArrayWrapper_get_flt(caw);
	self->platform =          CharArrayWrapper_get_uint8(caw);
	self->game =              CharArrayWrapper_get_nulltrm_str(caw);
	self->map_name =          CharArrayWrapper_get_nulltrm_str(caw);
	self->skybox =            CharArrayWrapper_get_nulltrm_str(caw);
	self->server_name =       CharArrayWrapper_get_nulltrm_str(caw);
	self->replay =            CharArrayWrapper_get_bit(caw);
}

#ifndef NO_PYTHON
PyObject *ServerInfo_to_PyDict(ServerInfo *self) {
	PyObject *sinfo_dict;
	PyObject *sinfo[16];
	PyObject *tmp;
	bool failed = false;

	if (self == NULL) {
		Py_INCREF(Py_None);
		return Py_None;
	}

	sinfo_dict = PyDict_New();
	if (sinfo_dict == NULL) {
		return NULL;
	}

	sinfo[0]  = PyLong_FromLong(self->version);
	sinfo[1]  = PyLong_FromLong(self->server_count);
	sinfo[2]  = PyBool_FromLong(self->stv);
	sinfo[3]  = PyBool_FromLong(self->dedicated);
	sinfo[4]  = PyLong_FromLong(self->max_crc);
	sinfo[5]  = PyLong_FromLong(self->max_classes);
	tmp       = PyBytes_FromStringAndSize(self->map_hash, 16);
	if (tmp != NULL) {
		sinfo[6]  = PyUnicode_FromEncodedObject(tmp, "cp437", NULL);
		Py_DECREF(tmp);
	} else {
		sinfo[6] = NULL;
	}
	sinfo[7]  = PyLong_FromLong(self->player_count);
	sinfo[8]  = PyLong_FromLong(self->max_player_count);
	sinfo[9]  = PyFloat_FromDouble(self->interval_per_tick);
	sinfo[10] = PyLong_FromLong(self->platform);
	sinfo[11] = PyUnicode_FromString(self->game);
	sinfo[12] = PyUnicode_FromString(self->map_name);
	sinfo[13] = PyUnicode_FromString(self->skybox);
	sinfo[14] = PyUnicode_FromString(self->server_name);
	sinfo[15] = PyBool_FromLong(self->replay);

	for (uint8_t i = 0; i < 16; i++) {
		if (sinfo[i] == NULL) { // Python conversion failure, error raised already
			failed = true;
			continue;
		}
		if (PyDict_SetItem(sinfo_dict, CONSTANTS_DICT_NAMES_ServerInfo->py_strings[i], sinfo[i]) < 0) {
			failed = true;
		}
		Py_DECREF(sinfo[i]);
	}

	if (failed) {
		Py_DECREF(sinfo_dict);
		return NULL;
	}

	return sinfo_dict;

}
#endif
