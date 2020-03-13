"""
Incomplete and tf2_dem_py-specific wrapper for the cJSON library.
"""

cdef extern from "cJSON.h" nogil:

	cdef struct cJSON:
		pass

	ctypedef int cJSON_bool

	const char *cJSON_Version()

	cJSON *cJSON_CreateObject()

	cJSON *cJSON_AddNullToObject(cJSON * const json_object, const char * const name)
	cJSON *cJSON_AddTrueToObject(cJSON * const json_object, const char * const name)
	cJSON *cJSON_AddFalseToObject(cJSON * const json_object, const char * const name)
	cJSON *cJSON_AddBoolToObject(cJSON * const json_object, const char * const name, const cJSON_bool boolean)
	cJSON *cJSON_AddNumberToObject(cJSON * const json_object, const char * const name, const double number)
	cJSON *cJSON_AddStringToObject(cJSON * const json_object, const char * const name, const char * const string)
	cJSON *cJSON_AddVolatileStringRefToObject(cJSON * const json_object, const char * const name, const char * const string)
	cJSON *cJSON_AddRawToObject(cJSON * const json_object, const char * const name, const char * const raw)
	cJSON *cJSON_AddObjectToObject(cJSON * const json_object, const char * const name)
	cJSON *cJSON_AddArrayToObject(cJSON * const json_object, const char * const name)

	char *cJSON_Print(const cJSON *item)
	char *cJSON_PrintUnformatted(const cJSON *item)
	char *cJSON_PrintBuffered(const cJSON *item, int prebuffer, cJSON_bool fmt)
	cJSON_bool cJSON_PrintPreallocated(cJSON *item, char *buffer, const int length, const cJSON_bool format_)

	void cJSON_Delete(cJSON *item)
