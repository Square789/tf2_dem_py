#include <stdlib.h>

#include "tf2_dem_py/demo_parser/helpers.h"

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/constants.h"

uint8_t _generic_arraylist_size_check(size_t type_size, void *array_, size_t *array_cap, size_t *array_len) {
	uint8_t resized = 0;

	void *tmp_ptr;
	void **array = array_;
	if (*array == NULL) {
		*array = malloc(type_size * 2);
		if (*array == NULL) {
			return 2;
		}
		resized = 1;
		*array_cap = 2;
		*array_len = 0;
	}
	if (*array_cap == *array_len) {
		tmp_ptr = realloc(*array, type_size * (*array_cap) * 2);
		if (tmp_ptr == NULL) {
			return 2;
		}
		resized = 1;
		*array = tmp_ptr;
		*array_cap = (*array_cap) * 2;
	}
	return resized;
}

#ifndef NO_PYTHON
PyObject *PyBytes_FromCAWLen(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = (char *)CharArrayWrapper_get_chars(caw, len);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyBytes_FromStringAndSize(str, len);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWLen(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = (char *)CharArrayWrapper_get_chars(caw, len);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromStringAndSize(str, len);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWUpToNull(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = (char *)CharArrayWrapper_get_chars_up_to_null(caw, len);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromString(str);
	free(str);
	return pystr;
}

PyObject *PyUnicode_FromCAWNulltrm(CharArrayWrapper *caw) {
	PyObject *pystr;
	char *str = (char *)CharArrayWrapper_get_nulltrm_str(caw);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromString(str);
	free(str);
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
#endif
