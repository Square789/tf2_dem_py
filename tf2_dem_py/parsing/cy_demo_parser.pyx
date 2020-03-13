from libc.stdio cimport FILE, fopen, fread, fclose, ftell, printf
from libc.stdlib cimport malloc, free
from libc.stdint cimport uint8_t

from tf2_dem_py.parsing cimport header
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.parsing.packet.parse_any cimport parse_any

from tf2_dem_py.cJSON.cJSON_wrapper cimport (cJSON_CreateObject, cJSON_Version,
	cJSON_PrintUnformatted, cJSON_Delete)

import json
from time import time

ERR_STRINGS_P = (
	"See CharArrayWrapper error below.",
	"Unkown packet id encountered.",
	"File I/O error.",
	"Unexpected EOF.",
	"cJSON error. (Likely due to memory allocation failure.)",
	"Unknown message id encountered.",
)

ERR_STRINGS_CAW = (
	"Buffer too short to perform requested read operation.",
	"Memory allocation failed.",
	"Read error on CharArrayWrapper instantiation.",
	"Memory allocation failure on CharArrayWrapper instantiation.",
	"Buffer length discrepancy on CharArrayWrapper instantiation.",
)

cdef str format_parser_error(uint8_t f_byte, uint8_t caw_byte):
	mesg_p = [j for i, j in enumerate(ERR_STRINGS_P) if (f_byte & (1 << i) != 0)]
	mesg_c = [j for i, j in enumerate(ERR_STRINGS_CAW) if (caw_byte & (1 << i) != 0)] \
		if f_byte & 1 == 1 else None
	if mesg_c is not None:
		return ("\n    ".join(mesg_p) +
			"\n    ===CharArrayWrapper errors:===\n    " +
			"\n    ".join(mesg_c))
	else:
		return "\n    ".join(mesg_p)

class ParserError(Exception):
	pass

cdef class CyDemoParser():
	"""
	Demo parser class.

	Has a pointer to a ParserState struct `state`.
	Possesses cJSON object where demo data should be written to.
	"""
	# attrs in pxd

	def __cinit__(self, char *target_file):
		self.stream = fopen(target_file, "rb")

		self.state = <ParserState *>malloc(sizeof(ParserState))
		if self.state == NULL:
			raise MemoryError("Failed to alloc memory for ParserState")
		self.state.finished = 0 # Initialize state
		self.state.FAILURE = 0
		self.state.RELAYED_CAW_ERR = 0

		self.json_obj = cJSON_CreateObject()
		print((cJSON_Version()).decode("utf-8"))

	def __dealloc__(self):
		fclose(self.stream)
		free(self.state)
		self.state = NULL

	cpdef dict parse(self):
		cdef char *res_str

		start = time()

		header.parse(self.stream, self.state, self.json_obj)
		if self.state.FAILURE != 0:
			raise ParserError("Failed to read header, additional "
				"info: {}".format(format_parser_error(
					self.state.FAILURE, self.state.RELAYED_CAW_ERR)))

		while not self.state.finished: # Main parser loop
			parse_any(self.stream, self.state, self.json_obj)
			if self.state.FAILURE != 0:
				print("failure")
				break
				raise ParserError("Demo parser failed @ byte {}; {}".format(
					<int>ftell(self.stream), format_parser_error(
						self.state.FAILURE, self.state.RELAYED_CAW_ERR)))

		end = time()

		res_str = cJSON_PrintUnformatted(self.json_obj)
		cJSON_Delete(self.json_obj)
		if res_str == NULL:
			raise ParserError("JSON library failed turning json to string.")
		print("Took ", end - start)
		return json.loads(res_str.decode("utf_8"))
