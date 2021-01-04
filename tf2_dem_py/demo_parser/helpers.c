#include "tf2_dem_py/demo_parser/helpers.h"

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/constants.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

PyObject *PyBytes_FromCAWLen(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = CharArrayWrapper_get_chars(caw, len);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyBytes_FromStringAndSize(str, len);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWLen(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = CharArrayWrapper_get_chars(caw, len); // get_chars
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromStringAndSize(str, len);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWUpToNull(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = CharArrayWrapper_get_chars_up_to_null(caw, len); // get_chars_up_to_null
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromString(str);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWNulltrm(CharArrayWrapper *caw) {
	PyObject *pystr;
	char *str = CharArrayWrapper_get_nulltrm_str(caw); // get_nulltrm_str
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromString(str);
	free(str);
	//if (pystr == NULL) printf("%s\n", str);
 	return pystr;
}

PyObject *PyTuple_FromArrayTransfer(Py_ssize_t size, PyObject **array) {
	PyObject *tuple = PyTuple_New(size);
	uint8_t failed = 0;
	if (tuple == NULL) {
		goto err_notuple;
	}
	for (Py_ssize_t i = 0; i < size; i++) {
		if (array[i] == NULL) {
			failed = 1;
		}
		PyTuple_SET_ITEM(tuple, i, array[i]);
	}

	if (failed) goto err_foundNULL;
	return tuple;

err_foundNULL:
	Py_DECREF(tuple); // Should decref all references to elements in tuple.
	// Since tupledealloc uses Py_XDECREF, NULLptrs aren't a problem.
	return NULL;

err_notuple:
	for (Py_ssize_t i = 0; i < size; i++) {
		Py_XDECREF(array[i]);
	}
	return NULL;
}

PyObject *CreateDict_Strings(PyObject **keys, PyObject **values, size_t length) {
	uint8_t failed = 0;
	PyObject *dict = PyDict_New();
	if (dict == NULL) {
		return NULL;
	}
	for (size_t i = 0; i < length; i++) {
		if (values[i] == NULL) {
			failed = 1;
			continue;
		}
		if (PyDict_SetItem(dict, keys[i], values[i]) < 0) {
			failed = 1;
		}
		Py_DECREF(values[i]);
	}
	if (failed) return NULL;
	return dict;
}

PyObject *CompactTuple2_Create() {
	PyObject *last_list, *tuple;
	last_list = PyList_New(0);
	if (last_list == NULL) {
		goto error0;
	}
	tuple = PyTuple_New(2);
	if (tuple == NULL) {
		goto error1;
	}
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(tuple, CONSTANTS_COMPACT_TUPLE2_FIELD_NAMES_IDX, Py_None);
	PyTuple_SET_ITEM(tuple, CONSTANTS_COMPACT_TUPLE2_DATA_IDX, last_list); // Steals ref

	return tuple;

error1: Py_DECREF(last_list);
error0:
	return NULL;
}

PyObject *CompactTuple3_Create() {
	PyObject *last_list, *tuple;
	last_list = PyList_New(0);
	if (last_list == NULL) {
		goto error0;
	}
	tuple = PyTuple_New(3);
	if (tuple == NULL) {
		goto error1;
	}
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(tuple, CONSTANTS_COMPACT_TUPLE3_NAME_IDX, Py_None);
	Py_INCREF(Py_None);
	PyTuple_SET_ITEM(tuple, CONSTANTS_COMPACT_TUPLE3_FIELD_NAMES_IDX, Py_None);
	PyTuple_SET_ITEM(tuple, CONSTANTS_COMPACT_TUPLE3_DATA_IDX, last_list); // Steals ref

	return tuple;

error1: Py_DECREF(last_list);
error0:
	return NULL;
}

PyObject *CompactTuple2_Finalize(PyObject *comptup) {
	PyObject *dict = PyDict_New();
	if (dict == NULL) {
		goto error0;
	}
	if (PyTuple_Size(comptup) != 2) {
		goto error1;
	}
	if (PyDict_SetItem(dict, CONSTANTS_PYSTR_FIELD_NAMES, PyTuple_GetItem(comptup, CONSTANTS_COMPACT_TUPLE2_FIELD_NAMES_IDX)) < 0) {
		goto error1;
	}
	if (PyDict_SetItem(dict, CONSTANTS_PYSTR_DATA, PyTuple_GetItem(comptup, CONSTANTS_COMPACT_TUPLE2_DATA_IDX)) < 0) {
		goto error1;
	}
	Py_DECREF(comptup);
	return dict;

error1: Py_DECREF(dict);
error0:
	Py_DECREF(comptup);
	return NULL;
}

PyObject *CompactTuple3_Finalize(PyObject *comptup) {
	PyObject *dict = PyDict_New();
	if (dict == NULL) {
		goto error0;
	}
	if (PyTuple_Size(comptup) != 3) {
		goto error1;
	}
	if (PyDict_SetItem(dict, CONSTANTS_PYSTR_NAME, PyTuple_GetItem(comptup, CONSTANTS_COMPACT_TUPLE3_NAME_IDX)) < 0) {
		goto error1;
	}
	if (PyDict_SetItem(dict, CONSTANTS_PYSTR_FIELD_NAMES, PyTuple_GetItem(comptup, CONSTANTS_COMPACT_TUPLE3_FIELD_NAMES_IDX)) < 0) {
		goto error1;
	}
	if (PyDict_SetItem(dict, CONSTANTS_PYSTR_DATA, PyTuple_GetItem(comptup, CONSTANTS_COMPACT_TUPLE3_DATA_IDX)) < 0) {
		goto error1;
	}
	Py_DECREF(comptup);
	return dict;

error1: Py_DECREF(dict);
error0:
	Py_DECREF(comptup);
	return NULL;
}
