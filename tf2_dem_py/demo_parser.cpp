
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include <chrono>

#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.hpp"
#include "tf2_dem_py/parsing/demo_header.hpp"
#include "tf2_dem_py/parsing/packet/parse_any.hpp"

using ParserState::ParserState_c;

// setup.py will fail without this line
#define _tf2_dem_py__version__ "0.0.1"

static PyObject *__version__;
static PyObject *ParserError;
static PyObject *PARSER_ERRMSG[16];
static PyObject *CAW_ERRMSG[8];
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
	PyObject *fppos_str, *lmsg_str;
	PyObject *final_string = NULL;
	if (builder_list == NULL) goto error0;
	fppos_str = PyUnicode_FromFormat("%li", ftell(fp));
	if (fppos_str == NULL) goto error1;
	lmsg_str = PyUnicode_FromFormat("%li", parser_state->current_message);
	if (lmsg_str == NULL) goto error2;

	if (PyList_Append(builder_list, ERROR_INIT0) < 0) goto error3;
	if (PyList_Append(builder_list, fppos_str) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_INIT1) < 0) goto error3;
	if (PyList_Append(builder_list, lmsg_str) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_INIT2) < 0) goto error3;
	if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
	for (int i = 0; i < 16; i++) {
		if ((1 << i) & parser_state->FAILURE) {
			if (PyList_Append(builder_list, PARSER_ERRMSG[i]) < 0) goto error3;
			if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
		}
	}
	if (parser_state->FAILURE & 1) { // CAW Error
		if (PyList_Append(builder_list, ERROR_SEP_CAW) < 0) goto error3;
		for (int i = 0; i < 8; i++) {
			if ((1 << i) & parser_state->RELAYED_CAW_ERR) {
				if (PyList_Append(builder_list, CAW_ERRMSG[i]) < 0) goto error3;
				if (PyList_Append(builder_list, ERROR_LINESEP) < 0) goto error3;
			}
		}
	}
	final_string = PyUnicode_Join(EMPTY_STR, builder_list); // If this fails, NULL is returned anyways

error3: Py_DECREF(lmsg_str);
error2: Py_DECREF(fppos_str);
error1: Py_DECREF(builder_list);
error0:
	return final_string;
}

/* Raise a ParserError from given parser state and python interpreter state.
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
		char *demo_path;

		try {
			parser_state = new ParserState_c;
		} catch (std::bad_alloc& ba) {
			return PyErr_NoMemory();
		}
		parser_state->flags = self->flags;

		if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:parse", KWARGS,
				&demo_path)) {
			return NULL;
		} // Does demo_path not need manual deallocation?

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
			Py_DECREF(tmp);
		}
		if (self->flags & FLAGS.GAME_EVENTS) {
			tmp = PyList_New(0);
			if (tmp == NULL) {
				return PyErr_NoMemory();
			}
			PyDict_SetItemString(res_dict, "game_events", tmp);
			Py_DECREF(tmp);
		}

		// Open file
		demo_fp = fopen(demo_path, "rb");
		if (demo_fp == NULL) {
			PyErr_SetString(PyExc_FileNotFoundError, "Demo does not exist.");
			return NULL;
		}

		start_time = std::chrono::steady_clock::now();

		// Aaaaaand go
		parse_demo_header(demo_fp, parser_state, res_dict);
		if (parser_state->FAILURE != 0) {
			raise_parser_error(demo_fp, parser_state);
			goto error;
		}
		while (!parser_state->finished) {
			packet_parse_any(demo_fp, parser_state, res_dict);
			if (parser_state->FAILURE != 0) {
				raise_parser_error(demo_fp, parser_state);
				goto error;
			}
		}
		end_time = std::chrono::steady_clock::now();

		printf("Took %i microsecs.\n", std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time).count());

		fclose(demo_fp);

		return res_dict;
error:
	Py_DECREF(res_dict);
	return NULL;
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
	NEWLINE_STR = PyUnicode_FromString("\n");
	ERROR_LINESEP = PyUnicode_FromString("\n    ");
	ERROR_SEP_CAW = PyUnicode_FromString("\n  ===CharArrayWrapper errors:===\n    ");
	ERROR_INIT0 = PyUnicode_FromString("Last message id: ");
	ERROR_INIT1 = PyUnicode_FromString(", File handle offset ");
	ERROR_INIT2 = PyUnicode_FromString(" bytes. Errors:");

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
