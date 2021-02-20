#ifndef HELPERS__H_
#define HELPERS__H_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"
#include "tf2_dem_py/demo_parser/parser_state/parser_state.h"

// Will check an arbitrary-type arraylist and create it if it is NULL or expand it if its capacity equals its length.
// type_size  : sizeof(<type>) in the call, where <type> is the type the arraylist contains.
// **array    : Pointer to the array start.
//  *array_cap: Pointer to where the arraylist's capacity is stored, will be changed if a malloc or realloc occurrs.
//  *array_len: Pointer to where the arraylist's length is stored, will be changed if a malloc occurs.
// Returns -1 on memory failures, 0 on success.
uint8_t _generic_arraylist_size_check(size_t type_size, void **array, size_t *array_cap, size_t *array_len);

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
// NULL elements will count as failure and return NULL.
PyObject *PyTuple_FromArrayTransfer(Py_ssize_t size, PyObject **array);

// Create a dict, mapping the supplied keys to their values respectively.
// Decreases the refcount of all non-NULL PyObjects in values, even on failure condition.
// Sets FAILURE of supplied ParserState on failure, may return NULL on failure or dict.
PyObject *CreateDict_Strings(PyObject **keys, PyObject **values, size_t length) ;

#endif
