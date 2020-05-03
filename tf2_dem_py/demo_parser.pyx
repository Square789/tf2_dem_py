# distutils: language = c++

"""
Main parser class.
"""

from libc.stdio cimport FILE, fopen, fread, fclose, ftell, printf, rewind
from libc.stdlib cimport malloc, free
from libc.stdint cimport uint8_t, uint16_t

from cpython.exc cimport PyErr_CheckSignals

from tf2_dem_py.flags cimport FLAGS
#from tf2_dem_py.parsing.demo_header cimport parse_demo_header 
from tf2_dem_py.parsing.game_events cimport free_GameEventDefinitionArray
from tf2_dem_py.parsing.parser_state cimport ParserState, ERR
#from tf2_dem_py.parsing.packet.parse_any cimport parse_any

from tf2_dem_py.cJSON cimport (cJSON_CreateObject, cJSON_Version,
	cJSON_PrintUnformatted, cJSON_Delete, cJSON_AddArrayToObject, cJSON)

import json
from time import time

__version__ = "0.0.1"

ERR_STRINGS_P = (
	"See CharArrayWrapper error below.",#1
	"Unkown packet id encountered.",#2
	"File I/O error.",#4
	"Unexpected EOF.",#8
	"cJSON error. (Likely due to memory allocation failure.)",#16
	"Unknown message id encountered.",#32
	"Memory allocation failed.",#64
	"Unknown game event encountered.",#128
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

cdef class DemoParser():
	"""
	Demo parser class.

	Has a pointer to a ParserState struct `state`.
	Possesses cJSON object where demo data should be written to.
	"""
	# attrs in pxd

	def __cinit__(self, char *target_file, uint16_t flagnum):
		print("cinit")

		self.stream = fopen(target_file, "rb")

		self.state = <ParserState *>malloc(sizeof(ParserState))
		if self.state == NULL:
			raise MemoryError("Failed to alloc memory for ParserState.")
		self.state.flags = flagnum # Initialize state
		self.state.finished = 0
		self.state.FAILURE = 0
		self.state.RELAYED_CAW_ERR = 0
		self.state.tick = 0
		self.state.game_event_defs = NULL
		self.state.current_message_contains_senderless_chat = 0

		print("bleb")
		self.json_obj = cJSON_CreateObject()
		print("bleb")

		if self.state.flags & FLAGS.CHAT: # Chat should be included in result
			print("blebb")
			chatarray = cJSON_AddArrayToObject(self.json_obj, "chat")
			if chatarray == NULL:
				raise MemoryError("Failed to alloc memory for cJSON chat array.")

		if self.state.flags & FLAGS.GAME_EVENTS:
			ge_array = cJSON_AddArrayToObject(self.json_obj, "game_events")
			if ge_array == NULL:
				raise MemoryError("Failed to alloc memory for cJSON game event array.")
		print("bleb")

	def __dealloc__(self):
		print("dealloc")
		fclose(self.stream)
		free(self.state)
		self.state = NULL

	cdef void cleanup(self):
		print("cleanup")
		cJSON_Delete(self.json_obj)
		#free_GameEventDefinitionArray(self.state.game_event_defs)

	cpdef dict parse(self):
		print("parse")
		"""
		Parses the demo, returning a dict corresponding to the flags the demo
		parser was initialized with.
		"""
	# 	cdef char *res_str

		start = time()

		rewind(self.stream)

		# parse_demo_header(self.stream, self.state, self.json_obj)
		# if self.state.FAILURE != 0:
		# 	raise ParserError("Failed to read header, additional "
		# 		"info: {}".format(format_parser_error(
		# 			self.state.FAILURE, self.state.RELAYED_CAW_ERR)))

		# while not self.state.finished: # Main parser loop
		# 	parse_any(self.stream, self.state, self.json_obj)
		# 	if self.state.FAILURE != 0:
		# 		raise ParserError("Demo parser failed @ byte {}; {}".format(
		# 			<int>ftell(self.stream), format_parser_error(
		# 				self.state.FAILURE, self.state.RELAYED_CAW_ERR)))
		# 	PyErr_CheckSignals() # yes or no who knows

		end = time()
		printf("done.\n")

		res_str = cJSON_PrintUnformatted(self.json_obj)

		self.cleanup()

		if res_str == NULL:
			raise ParserError("cJSON library failed turning json to string.")
		print("Took ", end - start)
		return json.loads(res_str.decode("utf_8"))

cpdef void bleb():
	print("whocares")
