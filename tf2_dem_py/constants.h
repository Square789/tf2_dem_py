#ifndef TF2_DEM_PY_CONSTANTS__H
#define TF2_DEM_PY_CONSTANTS__H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

typedef struct {
	const char **strings;
	PyObject **py_strings;
	Py_ssize_t size;
} PyStringHolder;

PyStringHolder *PyStringHolder_new(const char **strings, Py_ssize_t size);
void PyStringHolder_destroy(PyStringHolder *self);

// TODO: Cache tuples? Behavior? Think, dude, thinK!
PyObject *PyStringHolder_getPyTuple(PyStringHolder *self);

// Initialize PyObjects and StringHolders, only call this when a python
// interpreter is running already. Previous to this, all of the
// more complex constants will be NULL.
int CONSTANTS_initialize();

void CONSTANTS__deallocate_stringholders();

// Deallocate and DECREF constants and StringHolders.
void CONSTANTS_deallocate();

// Deallocate and XDECREF constants and StringHolders.
void CONSTANTS_deallocate_safe();

// === Constant declarations === //

extern uint16_t CONSTANTS_MAX_GAME_EVENT_AMOUNT;
extern Py_ssize_t CONSTANTS_COMPACT_TUPLE2_FIELD_NAMES_IDX;
extern Py_ssize_t CONSTANTS_COMPACT_TUPLE2_DATA_IDX;
extern Py_ssize_t CONSTANTS_COMPACT_TUPLE3_FIELD_NAMES_IDX;
extern Py_ssize_t CONSTANTS_COMPACT_TUPLE3_NAME_IDX;
extern Py_ssize_t CONSTANTS_COMPACT_TUPLE3_DATA_IDX;
extern PyObject *CONSTANTS_PYSTR_FIELD_NAMES;
extern PyObject *CONSTANTS_PYSTR_DATA;
extern PyObject *CONSTANTS_PYSTR_NAME;

extern PyStringHolder *CONSTANTS_DICT_NAMES_SayText2;
extern PyStringHolder *CONSTANTS_DICT_NAMES_GameEvent;
extern PyStringHolder *CONSTANTS_DICT_NAMES_header;

#endif
