#include <stdio.h>
#include <time.h>

#define PY_SSIZE_T_CLEAN
#include "Python.h"

#if (PY_MAJOR_VERSION != 3) || (PY_MINOR_VERSION < 8)
#  if defined(_MSC_VER)
#    pragma NOTE("Possibly incompatible python version. Should be at least 3.8")
#  elif defined(__GNUC__)
#    warning "Possibly incompatible python version. Should be at least 3.8"
#  endif
#endif


#include "tf2_dem_py/flags/flags.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/demo_parser/data_structs/demo_header.h"
#include "tf2_dem_py/demo_parser/parser_state.h"
#include "tf2_dem_py/demo_parser/helpers.h"

#define FLOAT_CLOCKS_PER_SEC ((float)CLOCKS_PER_SEC)

// setup.py will fail without this line
#define _tf2_dem_py__version__ "0.0.1"

#define PARSER_ERRMSG_SIZE (sizeof(ParserState_errflag_t) * 8)
#define CAW_ERRMSG_SIZE (sizeof(CharArrayWrapper_err_t) * 8)
#define GIL_SLEEP_MS 100


static PyObject *__version__;
static PyObject *ParserError;
static PyObject *PyConversionError;
// Python strings corresponding to the ParserState error bits.
static PyObject *PARSER_ERRMSG[PARSER_ERRMSG_SIZE];
// Python strings corresponding to the CharArrayWrapper error bits.
static PyObject *CAW_ERRMSG[CAW_ERRMSG_SIZE];
// Python string: "\n"
static PyObject *PYSTR_NEWLINE;
// Python string: ""
static PyObject *PYSTR_EMPTY;
static PyObject *PYSTR_ERROR_LINESEP;
static PyObject *PYSTR_ERROR_SEP_CAW;
static PyObject *PYSTR_ERROR_INIT0;
static PyObject *PYSTR_ERROR_INIT1;
static PyObject *PYSTR_ERROR_INIT2;
// Python string: "data"
static PyObject *PYSTR_DATA;
// Python string: "name"
static PyObject *PYSTR_NAME;
// Python string: "fields"
static PyObject *PYSTR_FIELDS;
// Python string for python conversion error
static PyObject *PYSTR_CONV_ERR;

// Builds a python error string from the failure attribute
// of a ParserState, which will need to be DECREFed by the caller.
// Returns NULL on any sort of error, at which point the computer is probably on fire anyways.
static PyObject *build_error_message(FILE *fp, ParserState *parser_state) {
	PyObject *builder_list = PyList_New(0);
	PyObject *lmsg_str, *fppos_str;
	PyObject *final_string = NULL;

	if (builder_list == NULL) goto error0;
	lmsg_str = PyUnicode_FromFormat("%u", (unsigned int)parser_state->current_message);
	if (lmsg_str == NULL) goto error1;
	fppos_str = PyUnicode_FromFormat("%li", ftell(fp));
	if (fppos_str == NULL) goto error2;

	if (PyList_Append(builder_list, PYSTR_ERROR_INIT0) < 0) goto error3;
	if (PyList_Append(builder_list, lmsg_str) < 0) goto error3;
	if (PyList_Append(builder_list, PYSTR_ERROR_INIT1) < 0) goto error3;
	if (PyList_Append(builder_list, fppos_str) < 0) goto error3;
	if (PyList_Append(builder_list, PYSTR_ERROR_INIT2) < 0) goto error3;
	if (PyList_Append(builder_list, PYSTR_ERROR_LINESEP) < 0) goto error3;

	// Add standard parser errors
	for (size_t i = 0; i < PARSER_ERRMSG_SIZE; i++) {
		if ((1 << i) & parser_state->failure) {
			if (PyList_Append(builder_list, PARSER_ERRMSG[i]) < 0) goto error3;
			if (PyList_Append(builder_list, PYSTR_ERROR_LINESEP) < 0) goto error3;
		}
	}
	if (PyList_SetSlice(builder_list, PyList_Size(builder_list) - 1, PyList_Size(builder_list), NULL) < 0) goto error3;

	// Add additional CAW error info
	if (parser_state->failure & 1) { // CAW Error
		if (PyList_Append(builder_list, PYSTR_ERROR_SEP_CAW) < 0) goto error3;
		for (size_t i = 0; i < CAW_ERRMSG_SIZE; i++) {
			if ((1 << i) & parser_state->RELAYED_CAW_ERR) {
				if (PyList_Append(builder_list, CAW_ERRMSG[i]) < 0) goto error3;
				if (PyList_Append(builder_list, PYSTR_ERROR_LINESEP) < 0) goto error3;
			}
		}
		if (PyList_SetSlice(builder_list, PyList_Size(builder_list) - 1, PyList_Size(builder_list), NULL) < 0) goto error3;
	}

	final_string = PyUnicode_Join(PYSTR_EMPTY, builder_list); // If this fails, NULL is returned anyways

error3: Py_DECREF(fppos_str);
error2: Py_DECREF(lmsg_str);
error1: Py_DECREF(builder_list);
error0:
	return final_string;
}

static void raise_with_set_cause(PyObject *exc, PyObject *err_str) {
	PyObject *old_exc_type, *old_exc_value, *old_exc_traceback;
	PyObject *new_exc_type, *new_exc_value, *new_exc_traceback;
	PyErr_Fetch(&old_exc_type, &old_exc_value, &old_exc_traceback);
	PyErr_NormalizeException(&old_exc_type, &old_exc_value, &old_exc_traceback);

	Py_XDECREF(old_exc_type);
	Py_XDECREF(old_exc_traceback);

	PyErr_SetObject(exc, err_str);
	if (old_exc_value != NULL) { // If a python exception was set beforehand
		PyErr_Fetch(&new_exc_type, &new_exc_value, &new_exc_traceback);
		PyErr_NormalizeException(&new_exc_type, &new_exc_value, &new_exc_traceback);
		PyException_SetCause(new_exc_value, old_exc_value);
		PyErr_Restore(new_exc_type, new_exc_value, new_exc_traceback);
	}
}

// Raise a ParserError from given parser state.
// Will return NULL.
static PyObject *raise_parser_error(FILE *fp, ParserState *parser_state) {
	PyObject *err_str = build_error_message(fp, parser_state);
	if (err_str == NULL) {
		return PyErr_NoMemory();
	}

	raise_with_set_cause(ParserError, err_str);
	Py_DECREF(err_str);

	return NULL;
}

static PyObject *raise_converter_error() {
	raise_with_set_cause(PyConversionError, PYSTR_CONV_ERR);
	return NULL;
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
	event_name = GameEventDefinition_get_python_name(parser_state->game_event_defs + ge_idx);
	if (event_name == NULL) {
		Py_DECREF(event_dict);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	if (PyDict_SetItem(event_dict, PYSTR_NAME, event_name) < 0) {
		Py_DECREF(event_dict); Py_DECREF(event_name);
		parser_state->failure |= ParserState_ERR_PYDICT | ParserState_ERR_UNKNOWN;
		return NULL;
	}
	Py_DECREF(event_name);

	event_fields_tuple = GameEventDefinition_get_field_names(
		parser_state->game_event_defs +
		((GameEvent *)parser_state->game_events.ptr)[ge_idx].event_type
	);
	if (event_fields_tuple == NULL) {
		Py_DECREF(event_dict);
		parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	if (PyDict_SetItem(event_dict, PYSTR_FIELDS, event_fields_tuple) < 0) {
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
	if (PyDict_SetItem(event_dict, PYSTR_DATA, event_data_list) < 0) {
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
	PyObject *game_event_container;

	// Compact:
	// {
	//     26: {"fields": ["name", "info"], "data": [["foo", 42], ...]},
	//     27: ...,
	// }

	//Non-Compact:
	// [
	//     {"event_type": 26, "name": "foo", "info": 42}, {...}, ...
	// ]

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
		for (size_t ge_idx = 0; ge_idx < parser_state->game_events.len; ge_idx++) {
			event_id = PyLong_FromLong(
				((GameEvent *)parser_state->game_events.ptr)[ge_idx].event_type
			);
			if (event_id == NULL) {
				goto error1_mem;
			}
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
		game_event_container = PyList_New(parser_state->game_events.len);
		if (game_event_container == NULL) {
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			goto error0;
		}
	}

	PyObject *game_event_python_repr;
	PyObject *final_container;
	PyObject *event_id;
	for (size_t ge_idx = 0; ge_idx < parser_state->game_events.len; ge_idx++) {
		GameEvent *ge_ptr = ((GameEvent *)parser_state->game_events.ptr) + ge_idx;
		if (parser_state->flags & FLAGS_COMPACT_GAME_EVENTS) {
			// Could probably add some bounds checking, but I think something else would've broken
			// earlier if event_type was not in the game event defs.
			game_event_python_repr = GameEvent_to_compact_PyTuple(
				ge_ptr, parser_state->game_event_defs + ge_ptr->event_type
			);
			if (game_event_python_repr == NULL) {
				if (PyErr_Occurred()) {
					goto error1;
				} else {
					goto error1_mem;
				}
			}
			// Dig through the compact structure
			event_id = PyLong_FromLong(ge_ptr->event_type);
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
			final_container = PyDict_GetItem(final_container, PYSTR_DATA);
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
				ge_ptr, parser_state->game_event_defs + ge_ptr->event_type
			);
			if (game_event_python_repr == NULL) {
				if (PyErr_Occurred()) {
					goto error1;
				} else {
					goto error1_mem;
				}
			}
			// Don't decref game_event_python_repr
			PyList_SET_ITEM(final_container, ge_idx, game_event_python_repr);
		}
	}
	return game_event_container;

error1_mem: parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
error1: Py_DECREF(game_event_container);
error0:
	return NULL;
}

static PyObject *build_chat_container(ParserState *parser_state) {
	PyObject *res_list = PyList_New(parser_state->chat_messages.len);
	PyObject *chat_obj;
	if (res_list == NULL) {
		return NULL;
	}

	for (size_t i = 0; i < parser_state->chat_messages.len; i++) {
		if (parser_state->flags & FLAGS_COMPACT_CHAT) {
			chat_obj = ChatMessage_to_PyTuple(((ChatMessage **)parser_state->chat_messages.ptr)[i]);
		} else {
			chat_obj = ChatMessage_to_PyDict(((ChatMessage **)parser_state->chat_messages.ptr)[i]);
		}
		if (chat_obj == NULL) {
			Py_DECREF(res_list);
			parser_state->failure |= ParserState_ERR_MEMORY_ALLOCATION;
			return NULL;
		}
		PyList_SET_ITEM(res_list, i, chat_obj);
	}

	return res_list;
}

// Build a result dict from a parser state which should contain a valid combination
// of parsed data and flags.
// Returns NULL on any sort of failure. Assume MemoryError if no Python exception is set.
static PyObject *build_result_dict_from_parser_state(ParserState *parser_state) {
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

	// Server info
	tmp = ServerInfo_to_PyDict(parser_state->server_info);
	if (tmp == NULL) { goto error1; }
	if (PyDict_SetItemString(res_dict, "server_info", tmp) < 0) {
		Py_DECREF(tmp);
		goto error1;
	}
	Py_DECREF(tmp);

	// Print msg
	if (parser_state->print_msg == NULL) {
		Py_INCREF(Py_None);
		tmp = Py_None;
	} else {
		tmp = PyUnicode_FromString(parser_state->print_msg);
		if (tmp == NULL) { goto error1; }
	}

	if (PyDict_SetItemString(res_dict, "printmsg", tmp) < 0) {
		Py_DECREF(tmp);
		goto error1;
	}
	Py_DECREF(tmp);

	// Game events
	if (parser_state->flags & FLAGS_GAME_EVENTS) {
		tmp = build_game_event_container(parser_state);
		if (tmp == NULL) { goto error1; }
		if (PyDict_SetItemString(res_dict, "game_events", tmp) < 0) {
			Py_DECREF(tmp);
			goto error1;
		};
		Py_DECREF(tmp);
	}

	// Chat
	if (parser_state->flags & FLAGS_CHAT) {
		tmp = build_chat_container(parser_state);
		if (tmp == NULL) { goto error1; }
		if (PyDict_SetItemString(res_dict, "chat", tmp) < 0) {
			Py_DECREF(tmp);
			goto error1;
		}
		Py_DECREF(tmp);
	}

	return res_dict;

error1: Py_DECREF(res_dict);
error0:
	return NULL;
}

// === Start of DemoParser methods === //

typedef struct DemoParser_s {
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

	clock_t start_time, end_time, last_gil_acq, cur_clock;
	ParserState *parser_state;
	FILE *demo_fp;
	PyObject *res_dict;
	PyObject *demo_path;

	if (
		!PyArg_ParseTupleAndKeywords(
			args, kwargs, "O&:parse", KWARGS,
			PyUnicode_FSConverter, &demo_path
		)
	) {
		goto memerror0;
	}

	// - Variable setup - //
	// - Create and setup ParserState
	parser_state = ParserState_new();
	if (parser_state == NULL) {
		goto memerror1;
	}
	parser_state->flags = self->flags;

	PyThreadState *_save;
	Py_UNBLOCK_THREADS

	// Open file
	demo_fp = fopen(PyBytes_AsString(demo_path), "rb");
	if (demo_fp == NULL) {
		goto file_error;
	}

	start_time = last_gil_acq = clock();

	// Start parsing
	ParserState_read_demo_header(parser_state, demo_fp);
	if (parser_state->failure != 0) {
		goto parser_error;
	}

	while (!parser_state->finished) {
		ParserState_parse_packet(parser_state, demo_fp);
		if (parser_state->failure != 0) {
			goto parser_error;
		}
		cur_clock = clock();
		if (((float)(cur_clock - last_gil_acq) / (FLOAT_CLOCKS_PER_SEC / 1000.0f)) > GIL_SLEEP_MS) {
			last_gil_acq = cur_clock;
			Py_BLOCK_THREADS
			if (PyErr_CheckSignals() < 0) {
				goto interrupted_error;
			}
			Py_UNBLOCK_THREADS
		}
	}

	// Done parsing

	fclose(demo_fp);

	end_time = clock();
	printf("Parsing successful, took %.3f secs.\n", ((float)(end_time - start_time) / FLOAT_CLOCKS_PER_SEC));

	Py_BLOCK_THREADS

	res_dict = build_result_dict_from_parser_state(parser_state);
	if (res_dict == NULL) {
		raise_converter_error();
		goto memerror2;
	}

	printf("Result dict created!\n");

	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);

	return res_dict;

// Error opening demo file, parsing has not begun but parser_state exists, GIL not held, no python error set.
file_error:
	Py_BLOCK_THREADS
	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);
	PyErr_SetString(PyExc_FileNotFoundError, "Demo does not exist.");
	return NULL;

// ParserState's error flags non-null, GIL not held, no python error is set.
parser_error:
	Py_BLOCK_THREADS // Will only be jumped into when parsing and when GIL is released.
	raise_parser_error(demo_fp, parser_state);
	fclose(demo_fp);
	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);
	return NULL;

// Signal check returned -1, GIL held, python error is set
interrupted_error:
	fclose(demo_fp);
	ParserState_destroy(parser_state);
	Py_DECREF(demo_path);
	return NULL;

// Memory error, GIL held, demo file will be closed, Python error may be set
memerror2: ParserState_destroy(parser_state);
memerror1: Py_DECREF(demo_path);
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
	NULL,                                    // tp_getattr
	NULL,                                    // tp_setattr
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
	NULL,                                    // tp_vectorcall
#if (PY_MAJOR_VERSION == 3) && (PY_MINOR_VERSION == 8)
	NULL,                                    // tp_print (Thanks PSF love u xoxo)
#endif
};

// DECREFs all constants, local and global ones.
void m_demo_parser_free() {
	for (uint16_t i = 0; i < PARSER_ERRMSG_SIZE; i++) {
		Py_DECREF(PARSER_ERRMSG[i]);
	}
	for (uint16_t i = 0; i < CAW_ERRMSG_SIZE; i++) {
		Py_DECREF(CAW_ERRMSG[i]);
	}
	Py_DECREF(__version__);
	Py_DECREF(ParserError);
	Py_DECREF(PyConversionError);
	Py_DECREF(PYSTR_EMPTY);
	Py_DECREF(PYSTR_NEWLINE);
	Py_DECREF(PYSTR_ERROR_LINESEP);
	Py_DECREF(PYSTR_ERROR_SEP_CAW);
	Py_DECREF(PYSTR_ERROR_INIT0);
	Py_DECREF(PYSTR_ERROR_INIT1);
	Py_DECREF(PYSTR_ERROR_INIT2);
	Py_DECREF(PYSTR_DATA);
	Py_DECREF(PYSTR_NAME);
	Py_DECREF(PYSTR_FIELDS);
	Py_DECREF(PYSTR_CONV_ERR);
	CONSTANTS_deallocate();
}

// Uses Py_XDECREF for removal of only local constants, in case some
// of them may have not been initialized after a failure.
void m_demo_parser_free_safe() {
	for (uint16_t i = 0; i < PARSER_ERRMSG_SIZE; i++) {
		Py_XDECREF(PARSER_ERRMSG[i]);
	}
	for (uint16_t i = 0; i < CAW_ERRMSG_SIZE; i++) {
		Py_XDECREF(CAW_ERRMSG[i]);
	}
	Py_XDECREF(__version__);
	Py_XDECREF(ParserError);
	Py_XDECREF(PyConversionError);
	Py_XDECREF(PYSTR_EMPTY);
	Py_XDECREF(PYSTR_NEWLINE);
	Py_XDECREF(PYSTR_ERROR_LINESEP);
	Py_XDECREF(PYSTR_ERROR_SEP_CAW);
	Py_XDECREF(PYSTR_ERROR_INIT0);
	Py_XDECREF(PYSTR_ERROR_INIT1);
	Py_XDECREF(PYSTR_ERROR_INIT2);
	Py_XDECREF(PYSTR_DATA);
	Py_XDECREF(PYSTR_NAME);
	Py_XDECREF(PYSTR_FIELDS);
	Py_XDECREF(PYSTR_CONV_ERR);
}

// Initializes local constants; returns 0 on success, 1 on failure.
uint8_t initialize_local_constants() {
	__version__ = PyUnicode_FromString(_tf2_dem_py__version__);
	ParserError = PyErr_NewException("demo_parser.ParserError", NULL, NULL);
	PyConversionError = PyErr_NewException("demo_parser.PyConversionError", NULL, NULL);
	PYSTR_EMPTY = PyUnicode_FromStringAndSize("", 0);
	PARSER_ERRMSG[0]  = PyUnicode_FromStringAndSize("CharArrayWrapper error, see below.", 34);
	PARSER_ERRMSG[1]  = PyUnicode_FromStringAndSize("Unknown packet id encountered.", 30);
	PARSER_ERRMSG[2]  = PyUnicode_FromStringAndSize("I/O error.", 10);
	PARSER_ERRMSG[3]  = PyUnicode_FromStringAndSize("Unexpected end of file.", 23);
	PARSER_ERRMSG[4]  = PyUnicode_FromStringAndSize("Unknown message id encountered.", 31);
	PARSER_ERRMSG[5]  = PyUnicode_FromStringAndSize("Memory allocation failure.", 26);
	PARSER_ERRMSG[6]  = PyUnicode_FromStringAndSize("Unknown game event encountered.", 31);
	PARSER_ERRMSG[7]  = PyUnicode_FromStringAndSize("Python dict error (Likely memory error).", 40);
	PARSER_ERRMSG[8]  = PyUnicode_FromStringAndSize("Python list error (Likely memory error).", 40);
	PARSER_ERRMSG[9]  = PyUnicode_FromStringAndSize("Game event index higher than size of game event array.", 54);
	for (uint32_t i = 10; i < PARSER_ERRMSG_SIZE; i++) {
		Py_XINCREF(PYSTR_EMPTY);
		PARSER_ERRMSG[i] = PYSTR_EMPTY;
	}
	PARSER_ERRMSG[15] = PyUnicode_FromStringAndSize("Unknown error.", 14);
	CAW_ERRMSG[0] = PyUnicode_FromStringAndSize("Buffer too short.", 17);
	CAW_ERRMSG[1] = PyUnicode_FromStringAndSize("Memory allocation failed.", 25);
	CAW_ERRMSG[2] = PyUnicode_FromStringAndSize("I/O error when reading from file.", 33);
	CAW_ERRMSG[3] = PyUnicode_FromStringAndSize("Initialization failed due to memory error.", 42);
	CAW_ERRMSG[4] = PyUnicode_FromStringAndSize("Initialization failed due to odd file reading result (Premature EOF?)", 69);
	for (uint32_t i = 5; i < CAW_ERRMSG_SIZE; i++) {
		Py_XINCREF(PYSTR_EMPTY);
		CAW_ERRMSG[i] = PYSTR_EMPTY;
	}
	PYSTR_NEWLINE = PyUnicode_FromStringAndSize("\n", 1);
	PYSTR_ERROR_LINESEP = PyUnicode_FromStringAndSize("\n    ", 5);
	PYSTR_ERROR_SEP_CAW = PyUnicode_FromStringAndSize("\n  ===CharArrayWrapper errors:===\n    ", 38);
	PYSTR_ERROR_INIT0 = PyUnicode_FromStringAndSize("Last message id: ", 17);
	PYSTR_ERROR_INIT1 = PyUnicode_FromStringAndSize(", File handle offset ", 21);
	PYSTR_ERROR_INIT2 = PyUnicode_FromStringAndSize(" bytes. Errors:", 15);
	PYSTR_DATA = PyUnicode_FromStringAndSize("data", 4);
	PYSTR_NAME = PyUnicode_FromStringAndSize("name", 4);
	PYSTR_FIELDS = PyUnicode_FromStringAndSize("fields", 6);
	PYSTR_CONV_ERR = PyUnicode_FromStringAndSize("Failed to convert extracted data to Python objects.", 51);

	// Check if something failed
	for (uint16_t i = 0; i < PARSER_ERRMSG_SIZE; i++) {
		if (PARSER_ERRMSG[i] == NULL) {
			return 1;
		}
	}
	for (uint16_t i = 0; i < CAW_ERRMSG_SIZE; i++) {
		if (CAW_ERRMSG[i] == NULL) {
			return 1;
		}
	}
	if (__version__ == NULL || ParserError == NULL || PyConversionError == NULL || PYSTR_EMPTY == NULL ||
		PYSTR_NEWLINE == NULL || PYSTR_ERROR_LINESEP == NULL || PYSTR_ERROR_SEP_CAW == NULL ||
		PYSTR_ERROR_INIT0 == NULL || PYSTR_ERROR_INIT1 == NULL || PYSTR_ERROR_INIT2 == NULL || PYSTR_DATA == NULL ||
		PYSTR_NAME == NULL || PYSTR_FIELDS == NULL || PYSTR_CONV_ERR == NULL
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
	if (CONSTANTS_initialize() > 0) {
		goto error1;
	}

	// Init local constants
	if (initialize_local_constants() > 0) {
		goto error2;
	}

	if (PyModule_AddObject(module, "ParserError", ParserError) < 0) {
		goto error3;
	}

	if (PyModule_AddObject(module, "PyConversionError", PyConversionError) < 0) {
		goto error4;
	}

	Py_INCREF(&DemoParser_Type);
	if (PyModule_AddObject(module, "DemoParser", (PyObject *)&DemoParser_Type) < 0) {
		goto error5;
	}

	Py_INCREF(__version__);
	if (PyModule_AddObject(module, "__version__", __version__) < 0) {
		goto error6;
	}

	return module;

error6:
	Py_DECREF(__version__);
error5:
	Py_DECREF(&DemoParser_Type);
error4:
	Py_DECREF(PyConversionError);
error3:
	Py_DECREF(ParserError);
error2:
	m_demo_parser_free_safe();
error1:
	CONSTANTS_deallocate();
	Py_DECREF(module);
error0:
	return NULL;
}
