#ifndef HELPERS__HPP_
#define HELPERS__HPP_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdio.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"

/* Convenience function to create a Python unicode object from a
 * CharArrayWrapper, directly freeing the allocated string afterwards. */
inline PyObject *PyUnicode_FromCAWLen(CharArrayWrapper *caw, size_t len) {
	PyObject *pystr;
	char *str = caw->get_chars(len);
	if (str == NULL) {
		return NULL;
	}

	pystr = PyUnicode_FromStringAndSize(str, len);
	free(str);
	return pystr;
}

/* Convenience function to create a Python unicode object from a
 * CharArrayWrapper, directly freeing the allocated string afterwards. */
inline PyObject *PyUnicode_FromCAWNulltrm(CharArrayWrapper *caw) {
	PyObject *pystr;
	char *str = caw->get_nulltrm_str();
	if (str == NULL) {
		printf("hä dumm\n");
		return NULL;
	}

	pystr = PyUnicode_FromString(str);
	printf("hä dummmmm : %s\n", str);
	free(str);
	return pystr;
}


#endif
