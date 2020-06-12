#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/helpers.hpp"
#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"

#include "tf2_dem_py/parsing/message/various.hpp"

namespace MessageParsers {

void Empty::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {}
void Empty::skip(CharArrayWrapper *caw, ParserState *parser_state) {}


void File::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void File::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(4, 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(0, 1);
}


void NetTick::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	uint32_t tick = caw->get_uint32();
	uint16_t frame_time = caw->get_uint16();
	uint16_t std_dev = caw->get_uint16();

	parser_state->tick = tick;
}

void NetTick::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(8, 0);
}


void StringCommand::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void StringCommand::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(caw->dist_until_null(), 0);
}


void SetConVar::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void SetConVar::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t amt = caw->get_uint8();
	char *tmp1, *tmp2;
	for (uint16_t i = 0; i < amt; i++) {
		tmp1 = caw->get_nulltrm_str();
		tmp2 = caw->get_nulltrm_str();
		printf("%s ||| %s\n", tmp1, tmp2);
		//caw->skip(caw->dist_until_null(), 0);
		//caw->skip(caw->dist_until_null(), 0);
	}
}


void SigOnState::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void SigOnState::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(5, 0);
}


void Print::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	PyObject *py_str;

	printf("[printmsg parser]: CAW @%d\n", caw->get_pos_byte());
	py_str = PyUnicode_FromCAWNulltrm(caw);
	if (py_str == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		if (caw->ERRORLEVEL != 0) {
			parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
			parser_state->FAILURE |= ParserState_ERR.CAW;
		}
		return;
	}

	if (PyDict_SetItemString(root_dict, "printmsg", py_str) < 0) {
		parser_state->FAILURE |= ParserState_ERR.PYDICT;
	}

	Py_DECREF(py_str);
}

void Print::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	size_t dist = caw->dist_until_null();
	caw->skip(dist, 0); // no error checks, occurs in message parsing loop
}


void ServerInfo::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	static const char *SINFO_NAMES[16] = {
		"version", "server_count", "stv", "dedicated",
		"max_crc", "max_classes", "map_hash", "player_count",
		"max_player_count", "interval_per_tick", "platform",
		"game", "map_name", "skybox", "server_name", "replay",
	};
	PyObject *sinfo_dict = PyDict_New();
	if (sinfo_dict == NULL) {
		parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		return;
	}

	PyObject *sinfo[16];
	PyObject *tmp;

	sinfo[0]  = PyLong_FromLong(caw->get_uint16());
	sinfo[1]  = PyLong_FromLong(caw->get_uint32());
	sinfo[2]  = PyBool_FromLong(caw->get_bit());
	sinfo[3]  = PyBool_FromLong(caw->get_bit());
	sinfo[4]  = PyLong_FromLong(caw->get_uint32());
	sinfo[5]  = PyLong_FromLong(caw->get_uint16());
	tmp       = PyBytes_FromCAWLen(caw, 16);
	sinfo[6]  = PyUnicode_FromEncodedObject(tmp, "cp437", NULL);
	sinfo[7]  = PyLong_FromLong(caw->get_uint8());
	sinfo[8]  = PyLong_FromLong(caw->get_uint8());
	sinfo[9]  = PyFloat_FromDouble(caw->get_flt());
	sinfo[10] = PyLong_FromLong(caw->get_uint8());
	sinfo[11] = PyUnicode_FromCAWNulltrm(caw);
	sinfo[12] = PyUnicode_FromCAWNulltrm(caw);
	sinfo[13] = PyUnicode_FromCAWNulltrm(caw);
	sinfo[14] = PyUnicode_FromCAWNulltrm(caw);
	sinfo[15] = PyBool_FromLong(caw->get_bit());

	for (uint8_t i = 0; i < 16; i++) {
		if (sinfo[i] == NULL) { // Python conversion failure, error raised already
			parser_state->FAILURE |= ParserState_ERR.MEMORY_ALLOCATION;
		} else {
			if (PyDict_SetItemString(sinfo_dict, SINFO_NAMES[i], sinfo[i]) < 0) {
				parser_state->FAILURE |= ParserState_ERR.PYDICT;
			} // Move value into dict, then decrease refcount
			Py_DECREF(sinfo[i]);
		}
	}

	if (PyDict_SetItemString(root_dict, "server_info", sinfo_dict) < 0) {
		parser_state->FAILURE |= ParserState_ERR.PYDICT;
	}

	if (caw->ERRORLEVEL != 0) {
		parser_state->FAILURE |= ParserState_ERR.CAW;
		parser_state->RELAYED_CAW_ERR = caw->ERRORLEVEL;
	}
}

void ServerInfo::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(36, 2);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0);
	caw->skip(caw->dist_until_null(), 0); // Less overhead than a loop i guess
	caw->skip(0, 1);
}


void SetView::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void SetView::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 3);
}


void FixAngle::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void FixAngle::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(6, 1);
}


void BspDecal::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void BspDecal::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	uint8_t existing_coords[3] = {0, 0, 0};
	uint8_t i;
	for (i = 0; i < 3; i++) {
		existing_coords[i] = caw->get_bit();
	}
	for (i = 0; i < 3; i++) {
		if (existing_coords[i] == 1) {
			caw->get_bit_coord();
		} 
	}
	caw->skip(1, 1);
	if (caw->get_bit() == 1) {
		caw->skip(3, 0);
	}
	caw->skip(0, 1);
}


void Entity::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void Entity::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(2, 4);
	uint16_t length;
	caw->read_raw(&length, 1, 3);
	caw->skip(length / 8, length % 8);
}


void PreFetch::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void PreFetch::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(1, 6);
}


void GetCvarValue::parse(CharArrayWrapper *caw, ParserState *parser_state, PyObject *root_dict) {
	this->skip(caw, parser_state);
}

void GetCvarValue::skip(CharArrayWrapper *caw, ParserState *parser_state) {
	caw->skip(4, 0);
	caw->skip(caw->dist_until_null(), 0);
}

}
