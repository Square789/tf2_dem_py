from libc.stdio cimport FILE, fopen, fread, fclose

from tf2_dem_py.parsing.header cimport parse as hp

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
		print("check 4")
		self.out["header"] = h
		print("check 5")
		__import__("pprint").pprint(self.out)
		print("check 6")
		while not self.finished:
			print("blblblbl")
			self.finished = 1
