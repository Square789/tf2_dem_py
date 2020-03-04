from libc.stdio cimport FILE

cdef class CyDemoParser():
	cdef:
		public char finished
		FILE *stream
		dict out

	cpdef dict parse(self)
