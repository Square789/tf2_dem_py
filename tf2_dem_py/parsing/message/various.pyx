
from tf2_dem_py.char_array_wrapper.char_array_wrapper cimport *
from tf2_dem_py.cJSON.cJSON_wrapper cimport cJSON

cdef class Print():
	@staticmethod
	cdef skip(CharArrayWrapper *caw):

	@staticmethod
	cdef parse(CharArrayWrapper *caw, cJSON *json):