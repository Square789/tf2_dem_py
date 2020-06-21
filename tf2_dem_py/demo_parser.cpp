
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <time.h>

#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/parsing/demo_header.hpp"
#include "tf2_dem_py/parsing/packet/parse_any.hpp"

// setup.py will fail without this line
#define _tf2_dem_py__version__ "0.0.1"

static PyObject *__version__;
static PyObject *ParserError;
static PyObject *PARSER_ERRMSG[16];
static PyObject *CAW_ERRMSG[8];
static PyObject *EMPTY_STR;
static PyObject *ERROR_LINESEP;
static PyObject *CAW_ERROR_SEP;

/* Builds a python error string from the FAILURE attribute
 * of a ParserState, remember to DECREF that.
 * Returns NULL on any sort of error, at that point just crash the program whatever
 * */
static PyObject *build_error_message(ParserState *parser_state) {
	PyObject *parser_list = PyList_New(0);
	PyObject *caw_list = PyList_New(0);
	PyObject *tmp_str0;
	PyObject *tmp_str1;
	PyObject *tmp_tup;
	PyObject *final_str;
	if (parser_list == NULL) goto error5;
	if (caw_list == NULL) goto error4;
	for (uint8_t i = 0; i < 16; i++) {
		if ((1 << i) & parser_state->FAILURE) {
			if (PyList_Append(parser_list, PARSER_ERRMSG[i]) < 0) {
				goto error4;
			}
		}
	}
	if (parser_state->FAILURE & 1) { // CAW error
		for (uint8_t i = 0; i < 8; i++) {
			if ((1 << i) & parser_state->RELAYED_CAW_ERR) {
				if (PyList_Append(caw_list, CAW_ERRMSG[i]) < 0) {
					goto error4;
				}
			}
		}
		tmp_str0 = PyUnicode_Join(ERROR_LINESEP, parser_list);
		if (tmp_str0 == NULL) goto error3;
		tmp_str1 = PyUnicode_Join(ERROR_LINESEP, caw_list);
		if (tmp_str1 == NULL) goto error2;
		tmp_tup = PyTuple_Pack(3, tmp_str0, CAW_ERROR_SEP, tmp_str1);
		if (tmp_tup == NULL) goto error1;
		final_str = PyUnicode_Join(EMPTY_STR, tmp_tup);
		if (final_str == NULL) goto error0;
		Py_DECREF(tmp_str0); Py_DECREF(tmp_str1); Py_DECREF(tmp_tup);
	} else {
		final_str = PyUnicode_Join(ERROR_LINESEP, parser_list);
		if (final_str == NULL) goto error4;
	}
	Py_DECREF(parser_list); Py_DECREF(caw_list);
	return final_str;
error0:
	Py_DECREF(tmp_tup);
error1:
	Py_DECREF(tmp_str1);
error2:
	Py_DECREF(tmp_str0);
error3:
	Py_DECREF(caw_list);
error4:
	Py_DECREF(parser_list);
error5:
	return NULL;
}

/* Raise a ParserError from given parser state and python interpreter state.
 * Will return a PyObject NULL pointer.
 */
static PyObject *raise_parser_error(ParserState *parser_state) {
	PyObject *exc_type, *exc_value, *exc_traceback;
	PyObject *new_exc_type, *new_exc_value, *new_exc_traceback;
	PyObject *parser_error;
	PyObject *err_str;
	PyErr_Fetch(&exc_type, &exc_value, &exc_traceback);

	err_str = build_error_message(parser_state);
	if (err_str == NULL) {
		return PyErr_NoMemory();
	}

	PyErr_SetObject(ParserError, err_str);
	if (exc_type != NULL) { // If a python exception was set beforehand
		PyErr_Fetch(&new_exc_type, &new_exc_value, &new_exc_traceback);
		PyErr_NormalizeException(&new_exc_type, &new_exc_value, &new_exc_traceback);
		//PyException_SetContext(new_exc_value, exc_value);
		PyException_SetCause(new_exc_value, exc_value);
		PyErr_Restore(new_exc_type, new_exc_value, new_exc_traceback);
		//Py_DECREF(exc_type); Py_DECREF(exc_traceback);
	}

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

		ParserState *parser_state = new ParserState;
		FILE *demo_fp;
		PyObject *res_dict;
		PyObject *tmp;
		char *demo_path;

		PyObject *current_exception;

		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:parse", KWARGS,
				&demo_path)) {
			return NULL;
		} // Does demo_path not need manual deallocation?

		// Set up parsing/result storage facilities
		parser_state->current_message_contains_senderless_chat = 0;
		parser_state->FAILURE = 0;
		parser_state->finished = 0;
		parser_state->flags = self->flags;
		parser_state->game_event_defs = NULL;
		parser_state->RELAYED_CAW_ERR = 0;
		parser_state->tick = 0;

		res_dict = PyDict_New();
		if (res_dict == NULL) {
			return PyErr_NoMemory();
		}
		if (self->flags & FLAGS.CHAT) {
			tmp = PyList_New(0);
			if (tmp == NULL) {
				return PyErr_NoMemory();
			}
			PyDict_SetItemString(res_dict, "chat", tmp);
		}
		if (self->flags & FLAGS.GAME_EVENTS) {
			tmp = PyList_New(0);
			if (tmp == NULL) {
				return PyErr_NoMemory();
			}
			PyDict_SetItemString(res_dict, "game_events", tmp);
		}

		// Open file
		printf("boutta open %s\n", demo_path);
		demo_fp = fopen(demo_path, "rb");
		if (demo_fp == NULL) {
			PyErr_SetString(PyExc_FileNotFoundError, "Demo does not exist.");
			return NULL;
		}

		time_t start_time, end_time;
		time(&start_time);

		// Aaaaaand go
		parse_demo_header(demo_fp, parser_state, res_dict);
		if (parser_state->FAILURE != 0) {
			return raise_parser_error(parser_state);
		}
		while (!parser_state->finished) {
			packet_parse_any(demo_fp, parser_state, res_dict);
			if (parser_state->FAILURE != 0) {
				return raise_parser_error(parser_state);
			}
		}

		time(&end_time);
		printf("Took %f seconds.\n", difftime(end_time, start_time));

		fclose(demo_fp);

		return res_dict;
		/*PyObject *hm = PyUnicode_FromString("\12\74\233\24");
		if (PyErr_Occurred() == NULL) {
			printf("no error ocurred\n");
		} else {
			printf("pyerror\n");
		}
		if (hm == NULL) {
			printf("failure");
			return NULL;
		}
		printf("succ");

		return hm;*/ // Some garbage test code. if you see this i forgot to remove it
	}

	// End of DemoParser specific methods; MethodDefTable below

	static PyMethodDef MethodDefs[] = {
		{
			"parse",
			(PyCFunction)(PyCFunctionWithKeywords)DemoParser::parse,
			METH_VARARGS | METH_KEYWORDS,
			NULL
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


static PyModuleDef DemoParser_ModuleDef {
	PyModuleDef_HEAD_INIT,
	.m_name = "tf2_dem_py.demo_parser",
	.m_doc = NULL,
	.m_size = -1,
};


PyMODINIT_FUNC PyInit_demo_parser() {
	PyObject *module;

	if (PyType_Ready(&DemoParser_Type) < 0) {
		goto error0;
	}

	module = PyModule_Create(&DemoParser_ModuleDef);
	if (module == NULL) {
		goto error0;
	}

	// Initialize globals, no clue if this is the right way lol
	__version__ = PyUnicode_FromString(_tf2_dem_py__version__);
	ParserError = PyErr_NewException("demo_parser.ParserError", NULL, NULL);
	PARSER_ERRMSG[0] = PyUnicode_FromString("CharArrayWrapper error, see below");
	PARSER_ERRMSG[1] = PyUnicode_FromString("Unknown packet id encountered.");
	PARSER_ERRMSG[2] = PyUnicode_FromString("I/O error.");
	PARSER_ERRMSG[3] = PyUnicode_FromString("Unexpected end of file");
	PARSER_ERRMSG[4] = PyUnicode_FromString("Unknown message id encountered.");
	PARSER_ERRMSG[5] = PyUnicode_FromString("Memory allocation failure.");
	PARSER_ERRMSG[6] = PyUnicode_FromString("Unknown game event encountered.");
	PARSER_ERRMSG[7] = PyUnicode_FromString("Python dict error (Likely memory error)");
	PARSER_ERRMSG[8] = PyUnicode_FromString("Python list error (Likely memory error)");
	PARSER_ERRMSG[9] = PyUnicode_FromString("");
	PARSER_ERRMSG[10] = PyUnicode_FromString("");
	PARSER_ERRMSG[11] = PyUnicode_FromString("");
	PARSER_ERRMSG[12] = PyUnicode_FromString("");
	PARSER_ERRMSG[13] = PyUnicode_FromString("");
	PARSER_ERRMSG[14] = PyUnicode_FromString("");
	PARSER_ERRMSG[15] = PyUnicode_FromString("Unknown error.");
	CAW_ERRMSG[0] = PyUnicode_FromString("Buffer too short.");
	CAW_ERRMSG[1] = PyUnicode_FromString("Memory allocation failed.");
	CAW_ERRMSG[2] = PyUnicode_FromString("I/O error when reading from file.");
	CAW_ERRMSG[3] = PyUnicode_FromString("Initialization failed due to memory error.");
	CAW_ERRMSG[4] = PyUnicode_FromString("Initialization failed due to odd file reading result (Premature EOF?)");
	CAW_ERRMSG[5] = PyUnicode_FromString("");
	CAW_ERRMSG[6] = PyUnicode_FromString("");
	CAW_ERRMSG[7] = PyUnicode_FromString("");
	EMPTY_STR = PyUnicode_FromString("");
	ERROR_LINESEP = PyUnicode_FromString("\n    ");
	CAW_ERROR_SEP = PyUnicode_FromString("\n     ===CharArrayWrapper errors:===\n    ");

	if (PyModule_AddObject(module, "ParserError", ParserError) < 0) {
		goto error1;
	}

	Py_INCREF(&DemoParser_Type);
	if (PyModule_AddObject(module, "DemoParser", (PyObject *)&DemoParser_Type) < 0) {
		goto error2;
	}

	Py_INCREF(__version__);
	if (PyModule_AddObject(module, "__version__", __version__) < 0) {
		goto error3;
	}

	return module;
error3:
	Py_DECREF(__version__);
error2:
	Py_DECREF(&DemoParser_Type);
error1:
	Py_DECREF(ParserError);
	Py_DECREF(module);
error0:
	return NULL;
}
