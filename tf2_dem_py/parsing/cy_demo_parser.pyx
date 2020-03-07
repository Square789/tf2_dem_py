from libc.stdio cimport FILE, fopen, fread, fclose
from libc.stdlib cimport malloc, free

from tf2_dem_py.parsing cimport header
from tf2_dem_py.parsing.parser_state cimport ParserState
from tf2_dem_py.parsing.packet.parse_any cimport parse_any

class ParserError(Exception):
	pass

cdef class CyDemoParser():
	"""
	Demo parser class.

	Has a pointer to a ParserState struct `state`.
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

	def __dealloc__(self):
		fclose(self.stream)
		free(self.state)
		self.state = NULL

	def __init__(self, *_):
		self.out = {}

	cpdef dict parse(self):
		cdef dict h
		h = header.parse(self.stream, self.state)
		self.out["header"] = h
		if self.state.FAILURE != 0:
			raise ParserError("Failed to read header, Here's some "
				" numbers: CAW_{:0>8b}, ERR_{:0>8}".format(
				self.state.RELAYED_CAW_ERR, self.state.FAILURE))
		while not self.state.finished:
			parse_any(self.stream, self.state)
		return self.out
