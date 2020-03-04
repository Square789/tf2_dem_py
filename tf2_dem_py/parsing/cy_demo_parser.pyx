from libc.stdio cimport FILE, fopen, fread, fclose

from header cimport parse as hp

cdef class CyDemoParser():
	# attrs in pxd

	def __init__(self, char *target_file):
		print("Demo parser created")
		self.finished = 0
		self.stream = fopen(target_file, "r")
		self.out = {}

	cpdef parse(self):
		print("parse method called")
		cdef dict h
		h = hp(self.stream)
		self.out["header"] = h
		__import__("pprint").pprint(self.out)
		while not self.finished:
			print("blblblbl")
			self.finished = 1
