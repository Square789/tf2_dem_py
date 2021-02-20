#include "tf2_dem_py/demo_parser/helpers.h"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

#include "tf2_dem_py/demo_parser/message/various.h"

void Empty_parse(CharArrayWrapper *caw, ParserState *parser_state) {}
void Empty_skip(CharArrayWrapper *caw, ParserState *parser_state) {}


void File_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	File_skip(caw, parser_state);
}

void File_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 4, 0);
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	CharArrayWrapper_skip(caw, 0, 1);
}


void NetTick_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	uint32_t tick = CharArrayWrapper_get_uint32(caw);
	//uint16_t frame_time = caw->get_uint16();
	CharArrayWrapper_skip(caw, 2, 0);
	//uint16_t std_dev = caw->get_uint16();
	CharArrayWrapper_skip(caw, 2, 0);

	parser_state->tick = tick;
}

void NetTick_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 8, 0);
}


void StringCommand_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	StringCommand_skip(caw, parser_state);
}

void StringCommand_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}


void SetConVar_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SetConVar_skip(caw, parser_state);
}

void SetConVar_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t amt = CharArrayWrapper_get_uint8(caw);
	//char *tmp1, *tmp2;
	for (uint16_t i = 0; i < amt; i++) {
		// tmp1 = caw->get_nulltrm_str();
		// tmp2 = caw->get_nulltrm_str();
		// printf("%s ||| %s\n", tmp1, tmp2);
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	}
}


void SigOnState_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SigOnState_skip(caw, parser_state);
}

void SigOnState_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 5, 0);
}


void Print_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	// Temporary
	Print_skip(caw, parser_state);
	// PyObject *py_str;

	// py_str = PyUnicode_FromCAWNulltrm(caw);
	// if (py_str == NULL) {
	// 	parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 	if (caw->ERRORLEVEL != 0) {
	// 		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
	// 		parser_state->failure |= ParserState_ERR_CAW;
	// 	}
	// 	return;
	// }

	// if (PyDict_SetItemString(root_dict, "printmsg", py_str) < 0) {
	// 	parser_state->failure |= ParserState_ERR_PYDICT;
	// }

	// Py_DECREF(py_str);
}

void Print_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}


void ServerInfo_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	// Temporary
	ServerInfo_skip(caw, parser_state);
	// static const char *SINFO_NAMES[16] = {
	// 	"version", "server_count", "stv", "dedicated",
	// 	"max_crc", "max_classes", "map_hash", "player_count",
	// 	"max_player_count", "interval_per_tick", "platform",
	// 	"game", "map_name", "skybox", "server_name", "replay",
	// };
	// PyObject *sinfo_dict = PyDict_New();
	// if (sinfo_dict == NULL) {
	// 	parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 	return;
	// }

	// PyObject *sinfo[16];
	// PyObject *tmp;

	// sinfo[0]  = PyLong_FromLong(CharArrayWrapper_get_uint16(caw));
	// sinfo[1]  = PyLong_FromLong(CharArrayWrapper_get_uint32(caw));
	// sinfo[2]  = PyBool_FromLong(CharArrayWrapper_get_bit(caw));
	// sinfo[3]  = PyBool_FromLong(CharArrayWrapper_get_bit(caw));
	// sinfo[4]  = PyLong_FromLong(CharArrayWrapper_get_uint32(caw));
	// sinfo[5]  = PyLong_FromLong(CharArrayWrapper_get_uint16(caw));
	// tmp       = PyBytes_FromCAWLen(caw, 16);
	// sinfo[6]  = PyUnicode_FromEncodedObject(tmp, "cp437", NULL); Py_XDECREF(tmp);
	// sinfo[7]  = PyLong_FromLong(CharArrayWrapper_get_uint8(caw));
	// sinfo[8]  = PyLong_FromLong(CharArrayWrapper_get_uint8(caw));
	// sinfo[9]  = PyFloat_FromDouble(CharArrayWrapper_get_flt(caw));
	// sinfo[10] = PyLong_FromLong(CharArrayWrapper_get_uint8(caw));
	// sinfo[11] = PyUnicode_FromCAWNulltrm(caw);
	// sinfo[12] = PyUnicode_FromCAWNulltrm(caw);
	// sinfo[13] = PyUnicode_FromCAWNulltrm(caw);
	// sinfo[14] = PyUnicode_FromCAWNulltrm(caw);
	// sinfo[15] = PyBool_FromLong(CharArrayWrapper_get_bit(caw));

	// for (uint8_t i = 0; i < 16; i++) {
	// 	if (sinfo[i] == NULL) { // Python conversion failure, error raised already
	// 		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
	// 	} else {
	// 		if (PyDict_SetItemString(sinfo_dict, SINFO_NAMES[i], sinfo[i]) < 0) {
	// 			parser_state->failure |= ParserState_ERR_PYDICT;
	// 		} // Move value into dict, then decrease refcount
	// 		Py_DECREF(sinfo[i]);
	// 	}
	// }

	// if (PyDict_SetItemString(root_dict, "server_info", sinfo_dict) < 0) {
	// 	parser_state->failure |= ParserState_ERR_PYDICT;
	// }

	// if (caw->ERRORLEVEL != 0) {
	// 	parser_state->failure |= ParserState_ERR_CAW;
	// 	parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
	// }
}

void ServerInfo_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 36, 2);
	for (uint8_t _ = 0; _ < 4; _++) {
		CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
	}
	CharArrayWrapper_skip(caw, 0, 1);
}


void SetView_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	SetView_skip(caw, parser_state);
}

void SetView_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 3);
}


void FixAngle_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	FixAngle_skip(caw, parser_state);
}

void FixAngle_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 6, 1);
}


void BspDecal_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	BspDecal_skip(caw, parser_state);
}

void BspDecal_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t existing_coords[3] = {0, 0, 0};
	uint8_t i;
	for (i = 0; i < 3; i++) {
		existing_coords[i] = CharArrayWrapper_get_bit(caw);
	}
	for (i = 0; i < 3; i++) {
		if (existing_coords[i] == 1) {
			CharArrayWrapper_get_bit_coord(caw);
		} 
	}
	CharArrayWrapper_skip(caw, 1, 1);
	if (CharArrayWrapper_get_bit(caw) == 1) {
		CharArrayWrapper_skip(caw, 3, 0);
	}
	CharArrayWrapper_skip(caw, 0, 1);
}


void Entity_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	Entity_skip(caw, parser_state);
}

void Entity_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 2, 4);
	uint16_t length;
	CharArrayWrapper_read_raw(caw, &length, 1, 3);
	CharArrayWrapper_skip(caw, length / 8, length % 8);
}


void PreFetch_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	PreFetch_skip(caw, parser_state);
}

void PreFetch_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 1, 6);
}


void GetCvarValue_parse(CharArrayWrapper *caw, ParserState *parser_state) {
	GetCvarValue_skip(caw, parser_state);
}

void GetCvarValue_skip(CharArrayWrapper *caw, ParserState *parser_state) {
	CharArrayWrapper_skip(caw, 4, 0);
	CharArrayWrapper_skip(caw, CharArrayWrapper_dist_until_null(caw), 0);
}
