from libc.stdlib cimport malloc, free
from libc.stdint cimport uint32_t, uint8_t
from libc.stdio cimport FILE, fread
from libc.string cimport memcpy

cdef class CharArrayWrapper:
	"""
	Class to read a block of data out of a file and offer functions
	to manipulate and further read it.
	"""
	# attrs in pxd

	def __cinit__(self):
		self.mem_ptr = NULL
		self.mem_len = 0
		self.bitbuf = 0
		self.bitbuf_len = 0
		self.pos = 0

	def __dealloc__(self):
		free(self.mem_ptr)

	@staticmethod
	cdef inline CharArrayWrapper create_new(FILE *file_ptr, uint32_t read_len):
		"""
		Create a new CharArrayWrapper and return it. This has to be done
		here as cinit only takes python arguments.

		See https://cython.readthedocs.io/en/latest/src/userguide/
			extension_types.html#instantiation-from-existing-c-c-pointers
		"""
		print("create_new called")
		cdef CharArrayWrapper caw = CharArrayWrapper.__new__(CharArrayWrapper)
		print("check")
		caw.mem_ptr = <uint8_t *>malloc(read_len)
		caw.mem_len = read_len
		caw.bitbuf = 0x00
		caw.bitbuf_len = 0
		caw.pos = 0
		if caw.mem_ptr == NULL:
			raise MemoryError("Failed to allocate memory for demo datachunk "
				"of size {}.".format(<int>read_len))
		if fread(&caw.mem_ptr, read_len, 1, file_ptr) == 0:
			raise IOError("File read operation failed or EOF was hit.")
		print("check 2")
		return caw

	cdef uint32_t get_next_str_size(self):
		"""
		Returns the amount of chars until the next null character is hit.
		(Excluding the null character)
		"""
		cdef uint32_t i = 0
		while True:
			if self.mem_ptr[i + self.pos] == 0x00:
				break
			if self.pos + i == self.buf_len:
				break
			i += 1
		return i

	cdef str read_next_utf8_str(self):
		"""
		Returns a python string with all chars up until the next null char
		converted to utf-8. 

		May raise: MemoryError
		"""
		cdef uint8_t *tmp
		tmp = self.read_raw(self.get_next_str_size())
		cdef str tmp_str = (tmp).decode("utf-8")
		free(tmp)
		return tmp_str

	cdef str read_utf8_str(self, uint32_t req_len):
		"""
		Returns a python string with length req_len.

		May raise: MemoryError
		"""
		cdef uint8_t *tmp
		tmp = self.read_raw(req_len)
		cdef str tmp_str = (tmp).decode("utf-8")
		free(tmp)
		return tmp_str

	cdef uint8_t *read_raw(self, uint32_t req_len):
		"""
		Returns a pointer to an array of unsigned chars
		corresponding to all chars from self.pos to self.pos + req_len.
		Array has to be freed.
		"""
		cdef uint8_t *tmp
		tmp = <uint8_t *>malloc(req_len)
		if tmp == NULL:
			raise MemoryError("Failed to allocate memory for return array "
				"of size {}.".format(<int>req_len))
		memcpy(<void *>tmp, <void *>(self.mem_ptr + self.pos), req_len)
		self.pos += req_len
		return tmp
