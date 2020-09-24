#ifndef TF2_DEM_PY_CONSTANTS__HPP
#define TF2_DEM_PY_CONSTANTS__HPP

#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace CONSTANTS {

class PyStringHolder {
	const char **strings;
public:
	PyObject **py_strings;
	Py_ssize_t size;

	/* Create a PyStringHolder and convert all Strings to PyUnicode objects.
	 * May raise std::bad_alloc. */
	PyStringHolder(const char **strings, Py_ssize_t size);
	~PyStringHolder();

	/* Create a tuple from stored py_strings. Reference counts won't be
	 * increased unless tuple creation fails, in which case NULL is returned. */
	PyObject *create_PyTuple();
};

/* Initialize PyObjects and StringHolders, only call this when a python
 * interpreter is running already. Previous to this, all of the
 * more complex constants  will be NULL. */
int initialize();

void _deallocate_stringholders();
/* Deallocate and DECREF constants and StringHolders. */
void deallocate();
/* Deallocate and XDECREF constants and StringHolders. */
void deallocate_safe();

extern uint16_t MAX_GAME_EVENT_AMOUNT;
extern Py_ssize_t COMPACT_TUPLE2_FIELD_NAMES_IDX;
extern Py_ssize_t COMPACT_TUPLE2_DATA_IDX;
extern Py_ssize_t COMPACT_TUPLE3_FIELD_NAMES_IDX;
extern Py_ssize_t COMPACT_TUPLE3_NAME_IDX;
extern Py_ssize_t COMPACT_TUPLE3_DATA_IDX;
extern PyObject *PYSTR_FIELD_NAMES;
extern PyObject *PYSTR_DATA;
extern PyObject *PYSTR_NAME;

extern PyStringHolder *DICT_NAMES_SayText2;
extern PyStringHolder *DICT_NAMES_GameEvent;
extern PyStringHolder *DICT_NAMES_header;

}

#endif
