#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "tf2_dem_py/constants.hpp"

PyObject* CONSTANTS::PARSER_ERRMSG[] = {
	PyUnicode_FromString("CharArrayWrapper error, see below"),
	PyUnicode_FromString("Unknown packet id encountered."),
	PyUnicode_FromString("I/O error."),
	PyUnicode_FromString("Unexpected end of file"),
	PyUnicode_FromString("Unknown message id encountered."),
	PyUnicode_FromString("Memory allocation failure."),
	PyUnicode_FromString("Unknown game event encountered."),
	PyUnicode_FromString("Python dict error (Likely memory error)"),
	PyUnicode_FromString("Python list error (Likely memory error)"),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString("Unknown error."),
};

PyObject* CONSTANTS::CAW_ERRMSG[] = { 
	PyUnicode_FromString("Buffer too short."),
	PyUnicode_FromString("Memory allocation failed."),
	PyUnicode_FromString("I/O error when reading from file."),
	PyUnicode_FromString("Initialization failed due to memory error."),
	PyUnicode_FromString("Initialization failed due to odd file reading result (Premature EOF?)"),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
	PyUnicode_FromString(""),
};

PyObject *CONSTANTS::EMPTY_STR = PyUnicode_FromString("");
PyObject *CONSTANTS::ERROR_LINESEP = PyUnicode_FromString("\n    ");
PyObject *CONSTANTS::CAW_ERROR_SEP = PyUnicode_FromString("\n     ===CharArrayWrapper errors:===\n    ");
