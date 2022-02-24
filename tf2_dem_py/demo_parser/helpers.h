#ifndef HELPERS__H_
#define HELPERS__H_

#ifndef NO_PYTHON
#  define PY_SSIZE_T_CLEAN
#  include <Python.h>
#endif

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"

typedef struct ArrayList_s {
	void *ptr;
	const size_t element_size;
	size_t len;
	size_t cap;
} ArrayList;

ArrayList *ArrayList_new(size_t element_size);
void ArrayList_init(ArrayList *self, size_t element_size);
void ArrayList_free(ArrayList *self);
void ArrayList_destroy(ArrayList *self);

// Will check an arraylist and create a buffer if its storage pointer is NULL or
// expand the buffer if its capacity equals its length.
// Returns 2 on failure, 1 when an allocation or reallocation occurred and 0 otherwise.
uint8_t ArrayList_check_size(ArrayList *self);

// Returns A pointer that allows the placement of one element in the ArrayList's
// storage buffer. Also increments the ArrayList's length by one.
// WARNING: If the capacity equals the length, this pointer may not belong to you.
// Always call `ArrayList_check_size` beforehand if you are unsure.
void *ArrayList_get_storage_pointer(ArrayList *self);


#ifndef NO_PYTHON
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
PyObject *CreateDict_Strings(PyObject **keys, PyObject **values, size_t length);
#endif

#endif
