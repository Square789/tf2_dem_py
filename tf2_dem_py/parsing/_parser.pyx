from libc.stdio cimport FILE, fopen, fread, fclose
cimport header
cimport packet

cdef class DemoParser():
	cdef public char finished
	cdef FILE *stream
	cdef dict out

	def __init__(self, char *target_file):
		self.finished = 0
		self.stream = fopen(target_file, "r")
		self.out = {}

	def parse(self):
		cdef dict h
		h = header.parse(self.stream)
		self.out["header"] = h
		__import__("pprint").pprint(self.out)
		while not self.finished:
			print("blblblbl")
			self.finished = 1
