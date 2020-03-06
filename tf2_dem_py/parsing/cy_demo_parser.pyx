from libc.stdio cimport FILE, fopen, fread, fclose

from tf2_dem_py.parsing.header cimport parse as hp

cdef class CyDemoParser():
	# attrs in pxd

	def __cinit__(self, char *target_file):
		self.stream = fopen(target_file, "r")

	def __dealloc__(self):
		fclose(self.stream)

	def __init__(self, *_):
		self.finished = 0
		self.out = {}

	cpdef dict parse(self):
		cdef dict h
		h = hp(self.stream)
		self.out["header"] = h
		while not self.finished:
			self.finished = 1
		return self.out
