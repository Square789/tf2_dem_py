#ifndef HELPERS__H_
#define HELPERS__H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

// Expands to a code block which will allocate array if it is NULL or reallocate the array
// to twice its capacity if array_len == array_cap.
// If any of those fail, it will return 1.
#define MACRO_ARRAYLIST_APPEND(type, array, array_cap, array_len) \
	type *_tmp_ptr; \
	if (array == NULL) { \
		array = (type *)malloc(sizeof(type) * 2); \
		array_cap = 2; \
		array_len = 0; \
		if (array == NULL) { return 1; } \
	} \
	if (array_len == array_cap) { \
		_tmp_ptr = (type *)realloc( \
			array, \
			sizeof(type) * array_cap * 2 \
		); \
		if (_tmp_ptr == NULL) { return 1; } \
		array = _tmp_ptr; \
		array_cap *= 2; \
	}

// Convenience function to create a Python bytes object from a
// CharArrayWrapper, directly freeing the allocated string afterwards.
PyObject *PyBytes_FromCAWLen(CharArrayWrapper *caw, size_t len);

// Convenience function to create a Python unicode object from a
// CharArrayWrapper, directly freeing the allocated string afterwards.
PyObject *PyUnicode_FromCAWLen(CharArrayWrapper *caw, size_t len);

// Convenience function to create a Python unicode object from a
// CharArrayWrapper, directly freeing the allocated string afterwards.
PyObject *PyUnicode_FromCAWUpToNull(CharArrayWrapper *caw, size_t len);

// Convenience function to create a Python unicode object from a
// CharArrayWrapper, directly freeing the allocated string afterwards.
PyObject *PyUnicode_FromCAWNulltrm(CharArrayWrapper *caw);

// Create a tuple from an array of PyObject pointers. All references will be
// stolen and transferred into the tuple or DECREFed if tuple creation fails.
// NULL elements will not cause a segfault, but count as failure and return NULL.
PyObject *PyTuple_FromArrayTransfer(Py_ssize_t size, PyObject **array);

// Create a dict, mapping the supplied keys to their values respectively.
// Decreases the refcount of all non-NULL PyObjects in values, even on failure condition.
// Sets FAILURE of supplied ParserState on failure, may return NULL on failure or dict.
PyObject *CreateDict_Strings(PyObject **keys, PyObject **values, size_t length) ;

// Create a "compact" tuple, which is a faster temporary way of accessing
// a field name list and a data list. Intended goal is to not access the same
// dict entry over and over again during data accumulation, speeding up writing.
// The last entry will be a list, the others Py_None.
// Returns NULL on failure.
PyObject *CompactTuple2_Create();

// Create a compact tuple just like CompactTuple2_Create, only containing three entries.
// The last one will be a list, the others Py_None.
// Returns NULL on failure.
PyObject *CompactTuple3_Create();

// Convert the compact tuple to a dict with the names "field_names" and "data",
// then return it. Will DECREF input tuple in any case.
// Returns NULL on failure.
PyObject *CompactTuple2_Finalize(PyObject *comptup);

// Convert the compact tuple to a dict with the names "name", "field_names" and "data",
// then return it. Will DECREF input tuple in any case.
// Returns NULL on failure.
PyObject *CompactTuple3_Finalize(PyObject *comptup);

#endif
