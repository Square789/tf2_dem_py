#include <stdio.h>

#include "tf2_dem_py/demo_parser/parser_state/data_structs/demo_header.h"

#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

DemoHeader *DemoHeader_new() {
	DemoHeader *self = (DemoHeader *)malloc(sizeof(DemoHeader));
	if (self == NULL) {
		return NULL;
	}
	DemoHeader_init(self);
	return self;
}

void DemoHeader_init(DemoHeader *self) {
	self->client_id = NULL;
	self->dem_prot = 0;
	self->frame_count = 0;
	self->game_dir = NULL;
	self->host_addr = NULL;
	self->ident = NULL;
	self->map_name = NULL;
	self->net_prot = 0;
	self->play_time = 0;
	self->sigon = 0;
	self->tick_count = 0;
}

void DemoHeader_destroy(DemoHeader *self) {
	free(self->client_id);
	free(self->game_dir);
	free(self->host_addr);
	free(self->ident);
	free(self->map_name);
	free(self);
}

PyObject *DemoHeader_to_PyDict(DemoHeader *self) {
	PyObject *header_dict = PyDict_New();
	uint8_t failed = 0;
	PyObject *header[11];

	if (header_dict == NULL) {
		return NULL;
	}

	header[0]  = PyUnicode_FromStringAndSize(self->ident, 8);
	header[1]  = PyLong_FromLong(self->net_prot);
	header[2]  = PyLong_FromLong(self->dem_prot);
	header[3]  = PyUnicode_FromString(self->host_addr);
	header[4]  = PyUnicode_FromString(self->client_id);
	header[5]  = PyUnicode_FromString(self->map_name);
	header[6]  = PyUnicode_FromString(self->game_dir);
	header[7]  = PyFloat_FromDouble(self->play_time);
	header[8]  = PyLong_FromLong(self->tick_count);
	header[9]  = PyLong_FromLong(self->frame_count);
	header[10] = PyLong_FromLong(self->sigon);

	for (uint8_t i = 0; i < 11; i++) {
		if (header[i] == NULL) { // Python conversion failure, error raised already
			failed = 1;
			continue;
		}
		if (PyDict_SetItem(header_dict, CONSTANTS_DICT_NAMES_header->py_strings[i], header[i]) < 0) {
			failed = 1;
		}
		Py_DECREF(header[i]);
	}

	if (failed) {
		Py_DECREF(header_dict);
		return NULL;
	}

	return header_dict;
}
