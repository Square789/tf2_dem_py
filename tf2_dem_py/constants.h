#ifndef TF2_DEM_PY_CONSTANTS__H
#define TF2_DEM_PY_CONSTANTS__H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct PyStringHolder_s {
	PyObject **py_strings;
	Py_ssize_t size;
} PyStringHolder;

PyStringHolder *PyStringHolder_new(const char **strings, Py_ssize_t size);
void PyStringHolder_destroy(PyStringHolder *self);

// Get all the StringHolder's strings in a tuple, you own this reference.
// Returns NULL on any sort of failure.
PyObject *PyStringHolder_getPyTuple(PyStringHolder *self);

// Initialize PyObjects and StringHolders, only call this when a python
// interpreter is running already. Previous to this, all of the
// more complex constants will be NULL.
int CONSTANTS_initialize();

// Deallocate and XDECREF constants and StringHolders.
void CONSTANTS_deallocate();

// === Constant declarations === //

extern PyStringHolder *CONSTANTS_DICT_NAMES_ChatMessage;
extern PyStringHolder *CONSTANTS_DICT_NAMES_GameEventContainer;
extern PyStringHolder *CONSTANTS_DICT_NAMES_DemoHeader;
extern PyStringHolder *CONSTANTS_DICT_NAMES_ServerInfo;

#endif
