#ifndef CONSTANTS__HPP_
#define CONSTANTS__HPP_

#define PY_SSIZE_T_CLEAN
#include <Python.h>

namespace CONSTANTS {

extern PyObject* PARSER_ERRMSG[];
extern PyObject* CAW_ERRMSG[];

extern PyObject *EMPTY_STR;
extern PyObject *ERROR_LINESEP;
extern PyObject *CAW_ERROR_SEP;

extern PyObject *ParserError;

}

#endif
