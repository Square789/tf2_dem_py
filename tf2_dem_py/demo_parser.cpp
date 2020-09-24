
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include <chrono>

#include "tf2_dem_py/flags/flags.hpp"
#include "tf2_dem_py/constants.hpp"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"
#include "tf2_dem_py/parsing/demo_header.hpp"
#include "tf2_dem_py/parsing/packet/parse_any.hpp"
#include "tf2_dem_py/parsing/helpers.hpp"

using ParserState::ParserState_c;

// setup.py will fail without this line
#define _tf2_dem_py__version__ "0.0.1"

#define demo_parser_PARSER_ERRMSG_SIZE 16
#define demo_parser_CAW_ERRMSG_SIZE 8

static PyObject *__version__;
static PyObject *ParserError;
static PyObject *PARSER_ERRMSG[demo_parser_PARSER_ERRMSG_SIZE];
static PyObject *CAW_ERRMSG[demo_parser_CAW_ERRMSG_SIZE];
static PyObject *NEWLINE_STR;
static PyObject *EMPTY_STR;
static PyObject *ERROR_LINESEP;
static PyObject *ERROR_SEP_CAW;
static PyObject *ERROR_INIT0;
static PyObject *ERROR_INIT1;
static PyObject *ERROR_INIT2;

/* Builds a python error string from the FAILURE attribute
 * of a ParserState, remember to DECREF that.
 * Returns NULL on any sort of error, at that point just crash the program whatever
 * */
static PyObject *build_error_message(FILE *fp, ParserState_c *parser_state) {
	PyObject *builder_list = PyList_New(0);
	PyObject *lmsg_str, *fppos_str;
	PyObject *final_string = NULL;
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
	for (int i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		if ((1 << i) & parser_state->FAILURE) {
			if (PyList_Append(builder_list, PARSER_ERRMSG[i]) < 0) goto error3;
			if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
		}
	}
	if (parser_state->FAILURE & 1) { // CAW Error
		if (PyList_Append(builder_list, ERROR_SEP_CAW) < 0) goto error3;
		for (int i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
			if ((1 << i) & parser_state->RELAYED_CAW_ERR) {
				if (PyList_Append(builder_list, CAW_ERRMSG[i]) < 0) goto error3;
				if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
			}
		}
	}
	final_string = PyUnicode_Join(EMPTY_STR, builder_list); // If this fails, NULL is returned anyways

error3: Py_DECREF(fppos_str);
error2: Py_DECREF(lmsg_str);
error1: Py_DECREF(builder_list);
error0:
	return final_string;
}

/* Raise a ParserError from given parser state.
 * Will return a PyObject NULL pointer.
 */
static PyObject *raise_parser_error(FILE *fp, ParserState_c *parser_state) {
	PyObject *err_str = build_error_message(fp, parser_state);
	if (err_str == NULL) {
		return PyErr_NoMemory();
	}

	PyErr_SetObject(ParserError, err_str); // Sets potentially existing exceptions as cause of this one, which is lovely
	return NULL;
}

namespace DemoParser {
	struct _struct {
		PyObject_HEAD
		uint32_t flags;
	};

	static PyObject *new_(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
		static char *KWARGS[] = {"flags", NULL};
		DemoParser::_struct *self;
		uint32_t flags;

		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "I:__new__", KWARGS, &flags)) {
			return NULL;
		}

		self = (DemoParser::_struct *)type->tp_alloc(type, 0);
		if (self == NULL) {
			return NULL;
		}

		// Set up Type variables
		self->flags = flags;

		return (PyObject *)self;
	}

	static void dealloc(DemoParser::_struct *self) {

	}

	// End of type methods; start of DemoParser specific methods

	static PyObject *parse(DemoParser::_struct *self, PyObject *args, PyObject *kwargs) {
		static char *KWARGS[] = {"path", NULL};

		std::chrono::steady_clock::time_point start_time, end_time;
		ParserState_c *parser_state;
		FILE *demo_fp;
		PyObject *res_dict;
		PyObject *tmp;
		PyObject *demo_path;

		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O&:parse", KWARGS,
				PyUnicode_FSConverter, &demo_path
		)) {
			goto memerror0;
		}

		// Open file
		demo_fp = fopen(PyBytes_AsString(demo_path), "rb");
		if (demo_fp == NULL) {
			goto file_not_found_error;
		}

		// - Variable setup - //
		// - Create and setup ParserState
		try {
			parser_state = new ParserState_c;
		} catch (std::bad_alloc& ba) {
			goto memerror1;
		}
		parser_state->flags = self->flags;

		// - Create top result dict
		res_dict = PyDict_New();
		if (res_dict == NULL) goto memerror2;

		// - Setup chat container
		if (self->flags & FLAGS::CHAT) {
			if (self->flags & FLAGS::COMPACT_CHAT) {
				parser_state->chat_container = CompactTuple2_Create();
			} else {
				parser_state->chat_container = PyList_New(0);
			}
			if (parser_state->chat_container == NULL) { goto memerror3; }
		}
		if (self->flags & FLAGS::COMPACT_CHAT) {
			PyTuple_SET_ITEM(
				parser_state->chat_container,
				CONSTANTS::COMPACT_TUPLE2_FIELD_NAMES_IDX,
				CONSTANTS::DICT_NAMES_SayText2->create_PyTuple()
			);
		}

		// - Setup game event container
		if (self->flags & FLAGS::GAME_EVENTS) {
			if (self->flags & FLAGS::COMPACT_GAME_EVENTS) {
				parser_state->game_event_container = PyDict_New();
			} else {
				parser_state->game_event_container = PyList_New(0);
			}
			if (parser_state->game_event_container == NULL) { goto memerror3; }
		}

		start_time = std::chrono::steady_clock::now();

		// Aaaaaand go
		parse_demo_header(demo_fp, parser_state, res_dict);
		if (parser_state->FAILURE != 0) {
			goto parser_error;
		}
		while (!parser_state->finished) {
			packet_parse_any(demo_fp, parser_state, res_dict);
			if (parser_state->FAILURE != 0) {
				goto parser_error;
			}
		}
		// Done

		fclose(demo_fp);

		// Set chat container as attribute of result dict
		if (parser_state->flags & FLAGS::CHAT) {
			if (parser_state->flags & FLAGS::COMPACT_CHAT) {
				parser_state->chat_container = CompactTuple2_Finalize(parser_state->chat_container);
				if (parser_state->chat_container == NULL) goto memerror3;
			}
			if (PyDict_SetItemString(res_dict, "chat", parser_state->chat_container) < 0) goto memerror3;
		}

		// Set game event container as attribute of result dict
		// If compact, finalize the comptups inside.
		if (parser_state->flags & FLAGS::GAME_EVENTS) {
			if (parser_state->flags & FLAGS::COMPACT_GAME_EVENTS) {
				Py_ssize_t pos = 0;
				PyObject *key, *value, *finalized_dict;
				while (PyDict_Next(parser_state->game_event_container, &pos, &key, &value)) {
					if (key == NULL || value == NULL) {
						parser_state->FAILURE |= ParserState::ERRORS::PYDICT | ParserState::ERRORS::MEMORY_ALLOCATION;
						goto memerror3;
					}
					Py_INCREF(value);
					finalized_dict = CompactTuple3_Finalize(value);
					if (finalized_dict == NULL) { parser_state->FAILURE |= ParserState::ERRORS::MEMORY_ALLOCATION; goto memerror3; }
					if (PyDict_SetItem(parser_state->game_event_container, key, finalized_dict) < 0) {
						Py_DECREF(finalized_dict);
						goto memerror3;
					}
					Py_DECREF(finalized_dict);
				}
			}
			if (PyDict_SetItemString(res_dict, "game_events", parser_state->game_event_container) < 0) goto memerror3;
		}

		end_time = std::chrono::steady_clock::now();
		printf("Parsing successful, took %i microsecs.\n",
			std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count());

		delete parser_state;
		Py_DECREF(demo_path);

		return res_dict;

	file_not_found_error:
		Py_DECREF(demo_path);
		PyErr_SetString(PyExc_FileNotFoundError, "Demo does not exist.");
		return NULL;

	parser_error:
		raise_parser_error(demo_fp, parser_state);
		fclose(demo_fp);
		Py_DECREF(res_dict);
		delete parser_state;
		Py_DECREF(demo_path);
		return NULL;

	memerror3: Py_DECREF(res_dict);
	memerror2: delete parser_state;
	memerror1: Py_DECREF(demo_path);
	memerror0:
		return PyErr_NoMemory();
}

	// === End of DemoParser specific methods; MethodDefTable below === //

	static PyMethodDef MethodDefs[] = {
		{
			"parse",
			(PyCFunction)(PyCFunctionWithKeywords)DemoParser::parse,
			METH_VARARGS | METH_KEYWORDS,
			NULL,
		},
		{NULL, NULL, 0, NULL},
	};

}

static PyTypeObject DemoParser_Type {
	PyVarObject_HEAD_INIT(NULL, 0)
	"tf2_dem_py.demo_parser.DemoParser",     // tp_name
	sizeof(DemoParser::_struct),             // tp_basicsize
	0,                                       // tp_itemsize
	(destructor)DemoParser::dealloc,         // tp_dealloc
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
	DemoParser::MethodDefs,                  // tp_methods
	NULL,                                    // tp_members
	NULL,                                    // tp_getset
	NULL,                                    // tp_base
	NULL,                                    // tp_dict
	NULL,                                    // tp_descr_get
	NULL,                                    // tp_descr_set
	0,                                       // tp_dictoffset
	NULL,                                    // tp_init
	NULL,                                    // tp_alloc
	DemoParser::new_,                        // tp_new
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

/* DECREFs all constants, local to this file and global ones. */
void m_free_demo_parser() {
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
	CONSTANTS::deallocate();
}

/* Uses Py_XDECREF for removal of only local constants, in case some
 * of them may have not been initialized after a failure. */
void free_demo_parser_safe() {
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
}

/* Initializes local constants; returns 0 on success, -1 on failure. */
int initialize_local_constants() {
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
		Py_INCREF(EMPTY_STR);
		CAW_ERRMSG[i] = EMPTY_STR;
	}
	NEWLINE_STR = PyUnicode_FromStringAndSize("\n", 1);
	ERROR_LINESEP = PyUnicode_FromStringAndSize("\n    ", 5);
	ERROR_SEP_CAW = PyUnicode_FromStringAndSize("\n  ===CharArrayWrapper errors:===\n    ", 38);
	ERROR_INIT0 = PyUnicode_FromStringAndSize("Last message id: ", 17);
	ERROR_INIT1 = PyUnicode_FromStringAndSize(", File handle offset ", 21);
	ERROR_INIT2 = PyUnicode_FromStringAndSize(" bytes. Errors:", 15);

	// Check if something failed
	for (uint16_t i = 0; i < demo_parser_PARSER_ERRMSG_SIZE; i++) {
		if (PARSER_ERRMSG[i] == NULL) return -1;
	}
	for (uint16_t i = 0; i < demo_parser_CAW_ERRMSG_SIZE; i++) {
		if (CAW_ERRMSG[i] == NULL) return -1;
	}
	if (__version__ == NULL || ParserError == NULL || EMPTY_STR == NULL || NEWLINE_STR == NULL ||
		ERROR_LINESEP == NULL || ERROR_SEP_CAW == NULL || ERROR_INIT0 == NULL || ERROR_INIT1 == NULL ||
		ERROR_INIT2 == NULL
	) {
		return -1;
	} 

	return 0;
}

static PyModuleDef DemoParser_ModuleDef {
	PyModuleDef_HEAD_INIT,
	"tf2_dem_py.demo_parser",      // m_name
	NULL,                          // m_doc
	-1,                            // m_size
	NULL,                          // m_methods
	NULL,                          // m_slots
	NULL,                          // m_traverse
	NULL,                          // m_clear
	(freefunc)m_free_demo_parser,  // m_free
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
	if (CONSTANTS::initialize() < 0) {
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
	Py_DECREF(module);
error2:
	free_demo_parser_safe();
error1:
	CONSTANTS::deallocate_safe();
error0:
	return NULL;
}
