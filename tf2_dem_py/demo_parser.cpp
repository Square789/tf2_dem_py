
#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>
#include <time.h>

#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/parsing/parser_state/parser_state.h"
#include "tf2_dem_py/parsing/demo_header.hpp"
#include "tf2_dem_py/parsing/packet/parse_any.hpp"

namespace DemoParser {
	struct _struct {
		PyObject_HEAD
		uint32_t flags;
	};

	static PyObject *new_(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
		DemoParser::_struct *self;
		self = (DemoParser::_struct *)type->tp_alloc(type, 0);
		if (self == NULL) {
			return NULL;
		}

		// Set up Type variables
		self->flags = 0;

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
			PyErr_Clear(); // Maybe integrate this error into parsererror if set?
			return PyErr_NoMemory(); // TODO: ADD CUSTOM MESSAGE HERE
		}
		while (!parser_state->finished) {
			packet_parse_any(demo_fp, parser_state, res_dict);
			if (parser_state->FAILURE != 0) {
				printf("!!! %d !!!\n", parser_state->FAILURE);
				PyErr_Clear(); // Maybe integrate into parsererror if set
				return PyErr_NoMemory(); // TODO: ADD CUSTOM MESSAGE HERE
			}	
		}

		time(&end_time);
		printf("Took %f seconds.\n", difftime(start_time, end_time));

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
		return NULL;
	}

	module = PyModule_Create(&DemoParser_ModuleDef);
	if (module == NULL) {
		return NULL;
	}

	Py_INCREF(&DemoParser_Type);
	if (PyModule_AddObject(module, "DemoParser", (PyObject *)&DemoParser_Type) < 0) {
		Py_DECREF(&DemoParser_Type); Py_DECREF(module);
		return NULL;
	}

	return module;
}
