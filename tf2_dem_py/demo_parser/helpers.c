#include <stdlib.h>

#include "tf2_dem_py/demo_parser/helpers.h"

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/constants.h"


uint8_t ArrayList_check_size(ArrayList *self) {
	uint8_t resized = 0;

	void *tmp_ptr;
	if (self->ptr == NULL) {
		self->ptr = malloc(self->element_size * 2);
		if (self->ptr == NULL) {
			return 2;
		}
		resized = 1;
		self->cap = 2;
		self->len = 0;
	}
	if (self->cap == self->len) {
		tmp_ptr = realloc(self->ptr, self->element_size * self->cap * 2);
		if (tmp_ptr == NULL) {
			return 2;
		}
		resized = 1;
		self->ptr = tmp_ptr;
		self->cap *= 2;
	}

	return resized;
}

void *ArrayList_get_storage_pointer(ArrayList *self) {
	void *ret = ((uint8_t *)self->ptr + (self->element_size * self->len));
	self->len += 1;
	return ret;
}

void ArrayList_init(ArrayList *self, size_t element_size) {
	self->ptr = NULL;
	*(size_t *)&self->element_size = element_size; // haha const cast-away go brrrrrr
	self->len = 0;
	self->cap = 0;
}

void ArrayList_destroy(ArrayList *self) {
	free(self->ptr);
	self->ptr = NULL;
	self->len = 0;
	self->cap = 0;
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
