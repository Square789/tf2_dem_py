from libc.stdio cimport FILE, fopen, fread, fclose

from tf2_dem_py.parsing cimport header
from tf2_dem_py.parsing.packet.parse_any cimport parse_any

cdef class CyDemoParser():
	# attrs in pxd

	def __cinit__(self, char *target_file):
		self.stream = fopen(target_file, "rb")
		self.finished = 0

	def __dealloc__(self):
		fclose(self.stream)

	def __init__(self, *_):
		self.out = {}

	cpdef dict parse(self):
		cdef dict h
		h = header.parse(self.stream)
		self.out["header"] = h
		while not self.finished:
			parse_any(self.stream, &self.finished)
		return self.out
