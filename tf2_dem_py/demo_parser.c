#include <stdio.h>
#include <time.h>

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"
#include "tf2_dem_py/demo_parser/packet/parse_any.h"
#include "tf2_dem_py/demo_parser/helpers.h"

// setup.py will fail without this line
#define _tf2_dem_py__version__ "0.0.1"

#define demo_parser_PARSER_ERRMSG_SIZE 16
#define demo_parser_CAW_ERRMSG_SIZE 8
#define demo_parser_GIL_SLEEP_MS 100

static PyObject *__version__;
static PyObject *ParserError;
// Python strings corresponding to the ParserState error bits.
static PyObject *PARSER_ERRMSG[demo_parser_PARSER_ERRMSG_SIZE];
// Python strings corresponding to the CharArrayWrapper error bits.
static PyObject *CAW_ERRMSG[demo_parser_CAW_ERRMSG_SIZE];
// Python string: "\n"
static PyObject *NEWLINE_STR;
// Python string: ""
static PyObject *EMPTY_STR;
static PyObject *ERROR_LINESEP;
static PyObject *ERROR_SEP_CAW;
static PyObject *ERROR_INIT0;
static PyObject *ERROR_INIT1;
static PyObject *ERROR_INIT2;
// Python string: "data"
static PyObject *DATA_STR;

// Builds a python error string from the FAILURE attribute
// of a ParserState, remember to DECREF that.
// Returns NULL on any sort of error, at that point just crash the program whatever
static PyObject *build_error_message(FILE *fp, ParserState *parser_state) {
	PyObject *builder_list = PyList_New(0);
	PyObject *lmsg_str, *fppos_str;
	PyObject *final_string = NULL;

	// FIXME: Can be called after fp is closed, which gets you a trip down UB-road
	if (builder_list == NULL) goto error0;
	lmsg_str = PyUnicode_FromFormat("%u", (unsigned int)parser_state->current_message);
	if (lmsg_str == NULL) goto error1;
	fppos_str = PyUnicode_FromFormat("%li", ftell(fp));
	if (fppos_str == NULL) goto error2;

	if (PyList_Append(builder_list, ERROR_INIT0) < 0) goto error3;
	if (PyList_Append(builder_list, lmsg_str) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_INIT1) < 0) goto error3;
	if (PyList_Append(builder_list, fppos_str) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_INIT2) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;

	// Add standard parser errors
	for (int i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		if ((1 << i) & parser_state->failure) {
			if (PyList_Append(builder_list, PARSER_ERRMSG[i]) < 0) goto error3;
			if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
		}
	}
	if (PyList_SetSlice(builder_list, PyList_Size(builder_list) - 1, PyList_Size(builder_list), NULL) < 0) goto error3;

	// Add additional CAW error info
	if (parser_state->failure & 1) { // CAW Error
		if (PyList_Append(builder_list, ERROR_SEP_CAW) < 0) goto error3;
		for (int i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
			if ((1 << i) & parser_state->RELAYED_CAW_ERR) {
				if (PyList_Append(builder_list, CAW_ERRMSG[i]) < 0) goto error3;
				if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
			}
		}
		if (PyList_SetSlice(builder_list, PyList_Size(builder_list) - 1, PyList_Size(builder_list), NULL) < 0) goto error3;
	}

	final_string = PyUnicode_Join(EMPTY_STR, builder_list); // If this fails, NULL is returned anyways

error3: Py_DECREF(fppos_str);
error2: Py_DECREF(lmsg_str);
error1: Py_DECREF(builder_list);
error0:
	return final_string;
}

// Builds the dict skeleton of a compact game event type like so:
// {"fields": ["name", "info"], "data": []}
// Returns NULL and modifies the ParserState's failure on any error.
static PyObject *build_compact_skeleton(ParserState *parser_state, size_t ge_idx) {
	PyObject *event_dict;
	PyObject *event_name;
	PyObject *event_fields_tuple;
	PyObject *event_data_list;
	event_dict = PyDict_New();
	if (event_dict == NULL) {
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	event_name = PyUnicode_FromString(
		parser_state->game_event_defs[parser_state->game_events[ge_idx]->event_type].name
	);
	if (event_name == NULL) {
		Py_DECREF(event_dict);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	if (PyDict_SetItemString(event_dict, "name", event_name) < 0) {
		Py_DECREF(event_dict); Py_DECREF(event_name);
		parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
		return NULL;
	}
	Py_DECREF(event_name);
	event_fields_tuple = GameEventDefinition_get_field_names(
		parser_state->game_event_defs + parser_state->game_events[ge_idx]->event_type
	);

	if (event_fields_tuple == NULL) {
		Py_DECREF(event_dict);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	if (PyDict_SetItemString(event_dict, "fields", event_fields_tuple) < 0) {
		Py_DECREF(event_dict); Py_DECREF(event_fields_tuple);
		parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
		return NULL;
	}
	Py_DECREF(event_fields_tuple);

	event_data_list = PyList_New(0);
	if (event_data_list == NULL) {
		Py_DECREF(event_dict);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	if (PyDict_SetItem(event_dict, DATA_STR, event_data_list) < 0) {
		Py_DECREF(event_dict); Py_DECREF(event_data_list);
		parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
		return NULL;
	}
	Py_DECREF(event_data_list);
	return event_dict;
}

// Builds the object that should be attached to the final result dict's "game_events" key, if present.
// Returns NULL and modifies ParserState's failure attribute  on any sort of failure.
static PyObject *build_game_event_container(ParserState *parser_state) {
	//Non-Compact:
	// [
	//     {"event_type": 26, "name": "foo", "info": 42}, {...}, ...
	// ]

	// Compact:
	// {
	//     26: {"fields": ["name", "info"], "data": [["foo", 42], ...]},
	//     27: ...,
	// }
	PyObject *game_event_container;
	if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
		// For compact mode, create a dict as above, "fields" already filled.
		PyObject *event_id;
		PyObject *event_dict;
		int key_memb_check;
		game_event_container = PyDict_New();
		if (game_event_container == NULL) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			goto error0;
		}
		for (size_t ge_idx = 0; ge_idx < parser_state->game_events_amount; ge_idx++) {
			event_id = PyLong_FromLong(parser_state->game_events[ge_idx]->event_type);
			if (event_id == NULL) { goto error1_mem; }
			key_memb_check = PyDict_Contains(game_event_container, event_id);
			if (key_memb_check == 0) {
				event_dict = build_compact_skeleton(parser_state, ge_idx);
				if (event_dict == NULL) {
					Py_DECREF(event_id);
					// No parser_state->failure manipulation; already done by build_compact_skeleton
					goto error1;
				}
				if (PyDict_SetItem(game_event_container, event_id, event_dict) < 0) {
					Py_DECREF(event_dict); Py_DECREF(event_id);
					parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
					goto error1;
				}
				Py_DECREF(event_dict);
			} else if (key_memb_check == -1) {
				Py_DECREF(event_id);
				parser_state->failure |= ParserState_ERR_UNKNOWN;
				goto error1;
			}
			Py_DECREF(event_id);
		}
	} else {
		// Just create a list.
		game_event_container = PyList_New(parser_state->game_events_amount);
		if (game_event_container == NULL) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			goto error0;
		}
	}

	PyObject *game_event_python_repr;
	PyObject *final_container;
	PyObject *event_id;
	for (size_t ge_idx = 0; ge_idx < parser_state->game_events_amount; ge_idx++) {
		if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
			// Could probably add some bounds checking, but I think something else would've broken
			// earlier if event_type was not in the game event defs.
			game_event_python_repr = GameEvent_to_compact_PyTuple(
				parser_state->game_events[ge_idx],
				parser_state->game_event_defs + parser_state->game_events[ge_idx]->event_type
			);
			if (game_event_python_repr == NULL) { goto error1_mem; }
			// Dig through the compact structure
			event_id = PyLong_FromLong(parser_state->game_events[ge_idx]->event_type);
			if (event_id == NULL) {
				Py_DECREF(game_event_python_repr);
				goto error1_mem;
			}
			final_container = PyDict_GetItem(game_event_container, event_id);
			if (final_container == NULL) {
				Py_DECREF(event_id); Py_DECREF(game_event_python_repr);
				parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
				goto error1;
			}
			Py_DECREF(event_id);
			final_container = PyDict_GetItem(final_container, DATA_STR);
			if (final_container == NULL) {
				Py_DECREF(game_event_python_repr);
				parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
				goto error1;
			}
			if (PyList_Append(final_container, game_event_python_repr) < 0) {
				Py_DECREF(game_event_python_repr);
				goto error1_mem;
			}
			Py_DECREF(game_event_python_repr);
		} else {
			final_container = game_event_container;
			game_event_python_repr = GameEvent_to_PyDict(
				parser_state->game_events[ge_idx],
				parser_state->game_event_defs + parser_state->game_events[ge_idx]->event_type
			);
			if (game_event_python_repr == NULL) {
				parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
				goto error1;
			}
			// Don't decref game_event_python_repr
			if (PyList_SET_ITEM(final_container, ge_idx, game_event_python_repr) < 0) {
				goto error1_mem;
			}
		}
	}
	return game_event_container;

error1_mem: parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
error1: Py_DECREF(game_event_container);
error0:
	return NULL;
}

// Build a result dict from a parser state which should contain a valid combination
// of parsed data and flags.
// Returns NULL on any sort of failure. Assume MemoryError.
static PyObject *build_result_dict_from_parser_state_and_flags(ParserState *parser_state, uint32_t flags) {
	PyObject *key, *value, *finalized_dict;
	PyObject *res_dict = PyDict_New();
	PyObject *tmp;
	if (res_dict == NULL) {
		goto error0;
	}

	// Demo header
	tmp = DemoHeader_to_PyDict(parser_state->demo_header);
	if (tmp == NULL) { goto error1; }
	if (PyDict_SetItemString(res_dict, "header", tmp) < 0) {
		Py_DECREF(tmp);
		goto error1;
	}
	Py_DECREF(tmp);

	// Game events
	tmp = build_game_event_container(parser_state);
	if (tmp == NULL) { goto error1; }
	if (PyDict_SetItemString(res_dict, "game_events", tmp) < 0) {
		Py_DECREF(tmp);
		goto error1;
	};
	Py_DECREF(tmp);

	return res_dict;

error1: Py_DECREF(res_dict);
error0:
	return NULL;
}

// Raise a ParserError from given parser state.
// Will return NULL.
static PyObject *raise_parser_error(FILE *fp, ParserState *parser_state) {
	PyObject *err_str = build_error_message(fp, parser_state);
	if (err_str == NULL) {
		return PyErr_NoMemory();
	}

	PyErr_SetObject(ParserError, err_str); // Sets potentially existing exceptions as cause of this one, which is lovely
	return NULL;
}

// === Start of DemoParser methods === //

typedef struct {
	PyObject_HEAD
	uint32_t flags;
} DemoParser;

static PyObject *DemoParser_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
	static char *KWARGS[] = {"flags", NULL};
	DemoParser *self;
	uint32_t flags;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I:__new__", KWARGS, &flags)) {
		return NULL;
	}

	self = (DemoParser *)type->tp_alloc(type, 0);
	if (self == NULL) {
		return NULL;
	}

	// Set up Type variables
	self->flags = flags;

	return (PyObject *)self;
}

//static void DemoParser_dealloc(DemoParser *self) {
//
//}

// End of type methods; start of DemoParser specific methods

static PyObject *DemoParser_parse(DemoParser *self, PyObject *args, PyObject *kwargs) {
	static char *KWARGS[] = {"path", NULL};

	clock_t start_time, end_time, last_gil_acq;
	ParserState *parser_state;
	FILE *demo_fp;
	PyObject *res_dict;
	PyObject *demo_path;

	if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:parse", KWARGS,
			PyUnicode_FSConverter, &demo_path
	)) { goto memerror0; }

	// Open file
	demo_fp = fopen(PyBytes_AsString(demo_path), "rb");
	if (demo_fp == NULL) {
		goto file_error;
	}

	// - Variable setup - //
	// - Create and setup ParserState
	parser_state = ParserState_new();
	if (parser_state == NULL) { goto memerror1; }
	parser_state->flags = self->flags;

	PyThreadState *_save;
	Py_UNBLOCK_THREADS

	start_time = last_gil_acq = clock();

	ParserState_read_DemoHeader(parser_state, demo_fp);
	if (parser_state->failure != 0) { Py_BLOCK_THREADS goto parser_error; }
	while (!parser_state->finished) {
		packet_parse_any(demo_fp, parser_state);
		if (parser_state->failure != 0) { Py_BLOCK_THREADS goto parser_error; }
	}
	// Done

	fclose(demo_fp);

	end_time = clock();
	printf("Parsing successful, took %f secs.\n", ((float)(end_time - start_time) / (float)CLOCKS_PER_SEC));

	Py_BLOCK_THREADS

	res_dict = build_result_dict_from_parser_state_and_flags(parser_state, self->flags);
	if (res_dict == NULL) {
		goto memerror2;
	}

	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);

	return res_dict;

file_error:
	Py_DECREF(demo_path);
	PyErr_SetString(PyExc_FileNotFoundError, "Demo does not exist.");
	return NULL;

parser_error:
	raise_parser_error(demo_fp, parser_state);
	fclose(demo_fp);
	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);
	return NULL;

memerror2:  ParserState_destroy(parser_state);
memerror1:
	Py_DECREF(demo_path);
memerror0:
	return (PyErr_Occurred() == NULL) ? PyErr_NoMemory() : NULL;
}

// === End of DemoParser specific methods; MethodDefTable below === //

static PyMethodDef DemoParser_MethodDefs[] = {
	{
		"parse",
		(PyCFunction)(PyCFunctionWithKeywords)DemoParser_parse,
		METH_VARARGS | METH_KEYWORDS,
		NULL,
	},
	{NULL, NULL, 0, NULL},
};


static PyTypeObject DemoParser_Type = {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tf2_dem_py.demo_parser.DemoParser",     // tp_name
	sizeof(DemoParser),                      // tp_basicsize
	0,                                       // tp_itemsize
	// (destructor)DemoParser_dealloc,
	NULL,                                    // tp_dealloc
	0,                                       // tp_vectorcall_offset
	NULL,                                    // tp_setattr
	NULL,                                    // tp_getattr
	NULL,                                    // tp_as_async
	NULL,                                    // tp_repr
	NULL,                                    // tp_as_number
	NULL,                                    // tp_as_sequence
	NULL,                                    // tp_as_mapping
	NULL,                                    // tp_hash
	NULL,                                    // tp_call
	NULL,                                    // tp_str
	NULL,                                    // tp_getattro
	NULL,                                    // tp_setattro
	NULL,                                    // tp_as_buffer
	Py_TPFLAGS_DEFAULT,                      // tp_flags
	NULL,                                    // tp_doc
	NULL,                                    // tp_traverse
	NULL,                                    // tp_clear
	NULL,                                    // tp_richcompare
	0,                                       // tp_weaklistoffset
	NULL,                                    // tp_iter
	NULL,                                    // tp_iternext
	DemoParser_MethodDefs,                   // tp_methods
	NULL,                                    // tp_members
	NULL,                                    // tp_getset
	NULL,                                    // tp_base
	NULL,                                    // tp_dict
	NULL,                                    // tp_descr_get
	NULL,                                    // tp_descr_set
	0,                                       // tp_dictoffset
	NULL,                                    // tp_init
	NULL,                                    // tp_alloc
	DemoParser_new,                          // tp_new
	NULL,                                    // tp_free
	NULL,                                    // tp_is_gc
	NULL,                                    // tp_bases
	NULL,                                    // tp_mro
	NULL,                                    // tp_cache
	NULL,                                    // tp_subclasses
	NULL,                                    // tp_weaklist
	NULL,                                    // tp_del
	0,                                       // tp_version_tag
	NULL,                                    // tp_finalize
};

// DECREFs all constants, local and global ones.
void m_demo_parser_free() {
	for (uint16_t i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		Py_DECREF(PARSER_ERRMSG[i]);
	}
	for (uint16_t i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
		Py_DECREF(CAW_ERRMSG[i]);
	}
	Py_DECREF(EMPTY_STR);
	Py_DECREF(NEWLINE_STR);
	Py_DECREF(ERROR_LINESEP);
	Py_DECREF(ERROR_SEP_CAW);
	Py_DECREF(ERROR_INIT0);
	Py_DECREF(ERROR_INIT1);
	Py_DECREF(ERROR_INIT2);
	Py_DECREF(DATA_STR);
	CONSTANTS_deallocate();
}

// Uses Py_XDECREF for removal of only local constants, in case some
// of them may have not been initialized after a failure.
void m_demo_parser_free_safe() {
	for (uint16_t i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		Py_XDECREF(PARSER_ERRMSG[i]);
	}
	for (uint16_t i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
		Py_XDECREF(CAW_ERRMSG[i]);
	}
	Py_XDECREF(EMPTY_STR);
	Py_XDECREF(NEWLINE_STR);
	Py_XDECREF(ERROR_LINESEP);
	Py_XDECREF(ERROR_SEP_CAW);
	Py_XDECREF(ERROR_INIT0);
	Py_XDECREF(ERROR_INIT1);
	Py_XDECREF(ERROR_INIT2);
	Py_XDECREF(DATA_STR);
}

// Initializes local constants; returns 0 on success, 1 on failure.
uint8_t initialize_local_constants() {
	__version__ = PyUnicode_FromString(_tf2_dem_py__version__);
	ParserError = PyErr_NewException("demo_parser.ParserError", NULL, NULL);
	EMPTY_STR = PyUnicode_FromStringAndSize("", 0);
	PARSER_ERRMSG[0] = PyUnicode_FromStringAndSize("CharArrayWrapper error, see below.", 34);
	PARSER_ERRMSG[1] = PyUnicode_FromStringAndSize("Unknown packet id encountered.", 30);
	PARSER_ERRMSG[2] = PyUnicode_FromStringAndSize("I/O error.", 10);
	PARSER_ERRMSG[3] = PyUnicode_FromStringAndSize("Unexpected end of file.", 23);
	PARSER_ERRMSG[4] = PyUnicode_FromStringAndSize("Unknown message id encountered.", 31);
	PARSER_ERRMSG[5] = PyUnicode_FromStringAndSize("Memory allocation failure.", 26);
	PARSER_ERRMSG[6] = PyUnicode_FromStringAndSize("Unknown game event encountered.", 31);
	PARSER_ERRMSG[7] = PyUnicode_FromStringAndSize("Python dict error (Likely memory error).", 40);
	PARSER_ERRMSG[8] = PyUnicode_FromStringAndSize("Python list error (Likely memory error).", 40);
	PARSER_ERRMSG[9] = PyUnicode_FromStringAndSize("Game event index higher than size of game event array.", 54);
	for (uint32_t i = 10; i < 15; i++) {
		Py_XINCREF(EMPTY_STR);
		PARSER_ERRMSG[i] = EMPTY_STR;
	}
	PARSER_ERRMSG[15] = PyUnicode_FromStringAndSize("Unknown error.", 14);
	CAW_ERRMSG[0] = PyUnicode_FromStringAndSize("Buffer too short.", 17);
	CAW_ERRMSG[1] = PyUnicode_FromStringAndSize("Memory allocation failed.", 25);
	CAW_ERRMSG[2] = PyUnicode_FromStringAndSize("I/O error when reading from file.", 33);
	CAW_ERRMSG[3] = PyUnicode_FromStringAndSize("Initialization failed due to memory error.", 42);
	CAW_ERRMSG[4] = PyUnicode_FromStringAndSize("Initialization failed due to odd file reading result (Premature EOF?)", 69);
	for (uint32_t i = 5; i < 8; i++) {
		Py_XINCREF(EMPTY_STR);
		CAW_ERRMSG[i] = EMPTY_STR;
	}
	NEWLINE_STR = PyUnicode_FromStringAndSize("\n", 1);
	ERROR_LINESEP = PyUnicode_FromStringAndSize("\n    ", 5);
	ERROR_SEP_CAW = PyUnicode_FromStringAndSize("\n  ===CharArrayWrapper errors:===\n    ", 38);
	ERROR_INIT0 = PyUnicode_FromStringAndSize("Last message id: ", 17);
	ERROR_INIT1 = PyUnicode_FromStringAndSize(", File handle offset ", 21);
	ERROR_INIT2 = PyUnicode_FromStringAndSize(" bytes. Errors:", 15);
	DATA_STR = PyUnicode_FromStringAndSize("data", 4);

	// Check if something failed
	for (uint16_t i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		if (PARSER_ERRMSG[i] == NULL) { return 1; }
	}
	for (uint16_t i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
		if (CAW_ERRMSG[i] == NULL) { return 1; }
	}
	if (__version__ == NULL || ParserError == NULL || EMPTY_STR == NULL || NEWLINE_STR == NULL ||
		ERROR_LINESEP == NULL || ERROR_SEP_CAW == NULL || ERROR_INIT0 == NULL || ERROR_INIT1 == NULL ||
		ERROR_INIT2 == NULL || DATA_STR == NULL
	) {
		return 1;
	} 

	return 0;
}

static PyModuleDef DemoParser_ModuleDef = {
	PyModuleDef_HEAD_INIT,
	"tf2_dem_py.demo_parser",          // m_name
	NULL,                              // m_doc
	-1,                                // m_size
	NULL,                              // m_methods
	NULL,                              // m_slots
	NULL,                              // m_traverse
	NULL,                              // m_clear
	(freefunc)m_demo_parser_free,      // m_free
};

// === PyInit === //

PyMODINIT_FUNC PyInit_demo_parser() {
	PyObject *module;

	if (PyType_Ready(&DemoParser_Type) < 0) {
		goto error0;
	}

	module = PyModule_Create(&DemoParser_ModuleDef);
	if (module == NULL) {
		goto error0;
	}

	// Init global constants
	if (CONSTANTS_initialize() < 0) {
		goto error1;
	}

	// Init local constants
	if (initialize_local_constants() < 0) {
		goto error2;
	}

	if (PyModule_AddObject(module, "ParserError", ParserError) < 0) {
		goto error3;
	}

	Py_INCREF(&DemoParser_Type);
	if (PyModule_AddObject(module, "DemoParser", (PyObject *)&DemoParser_Type) < 0) {
		goto error4;
	}

	Py_INCREF(__version__);
	if (PyModule_AddObject(module, "__version__", __version__) < 0) {
		goto error5;
	}

	return module;

error5:
	Py_DECREF(__version__);
error4:
	Py_DECREF(&DemoParser_Type);
error3:
	Py_DECREF(ParserError);
error2:
	m_demo_parser_free_safe();
error1:
	CONSTANTS_deallocate_safe();
	Py_DECREF(module);
error0:
	return NULL;
}
