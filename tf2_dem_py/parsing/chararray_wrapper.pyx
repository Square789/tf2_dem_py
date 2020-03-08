from libc.stdlib cimport malloc, free
from libc.stdint cimport uint8_t, uint32_t, uint64_t
from libc.stdio cimport FILE, fread, ferror, printf
from libc.string cimport memcpy, memset

cdef class CharArrayWrapper:
	"""
	Class to read a block of bytes out of a file and offer convenient
	methods to read it as if it were a continuous string of bits.

	Call create_new to instantiate a new CharArrayWrapper.

	ERRORLEVEL should be checked for deviation from 0 before processing
		retrieved data:
	0b 0 0 0 0 0 0 0 0
	         | | | | \\ Buffer too short to perform requested read operation.
	         | | | \\ Memory allocation failure.
	         | | \\ Instantiation failure, read error.
			 | \\ Instantiation failure, malloc error.
			 \\ Instantiation failure, amount of bytes read differs from requested amount.
	"""
	# attrs in pxd

	def __dealloc__(self):
		free(self.mem_ptr)
		self.mem_ptr = NULL

	@staticmethod
	cdef CharArrayWrapper create_new(FILE *file_ptr, size_t read_len):
		"""
		Create a new CharArrayWrapper and return it. This has to be done
		here as cinit only takes python arguments.

		*file_ptr: Pointer to a stdio.FILE struct.
		read_len: Bytes to be read from the supplied FILE.

		WARNING: Instantiated CharArrayWrapper may already be faulty

		See https://cython.readthedocs.io/en/latest/src/userguide/
			extension_types.html#instantiation-from-existing-c-c-pointers
		"""
		cdef CharArrayWrapper caw = CharArrayWrapper.__new__(CharArrayWrapper)
		cdef size_t fread_res
		caw.mem_ptr = <uint8_t *>malloc(read_len)
		caw.mem_len = read_len
		caw.bitbuf = 0x00
		caw.bitbuf_len = 0
		caw.pos = 0
		caw.ERRORLEVEL = 0
		if caw.mem_ptr == NULL:
			caw.ERRORLEVEL |= 0b1000
		fread_res = fread(caw.mem_ptr, sizeof(uint8_t), read_len, file_ptr)
		if ferror(file_ptr):
			caw.ERRORLEVEL |= 0b100
		if fread_res != read_len:
			caw.ERRORLEVEL |= 0b10000
		return caw

	cdef uint8_t _ver_buf_health(self, size_t req_bytes, uint8_t req_bits):
		"""
		Verify enough data is left that can still be read, counting from
			current bitbuffer length and position in char array
		Returns:
		0 if buffer is still large enough.
		1 if the buffer is too short.
		2 if more than 7 bits were requested.
		"""
		if req_bits > 7:
			return 2
		# Temp var in case bitbuf overflows
		cdef uint8_t tmp
		tmp = 1 if req_bits > self.bitbuf_len else 0
		if (self.mem_len - self.pos) < (req_bytes + tmp):
			return 1
		return 0

	cdef void _read_raw(self, void *target_ptr, size_t req_bytes, uint8_t req_bits):
		"""
		Copies Requested amount of bits and bytes to the supplied
		pointer. It is the caller's responsibility the pointer points to a
		block of memory large enough.
		If more data was requested than the buffer could handle, ERRORLEVEL's first
		bit will be set to 1 and no data will be written to the pointer.
		"""
		cdef uint8_t carry # temporary bit level storage
		cdef size_t i # Loop variable
		cdef void *tmp_ptr = target_ptr
		if self._ver_buf_health(req_bytes, req_bits) != 0:
			self.ERRORLEVEL |= 0b1
			return
		if self.bitbuf_len == 0:
			memcpy(target_ptr, <void *>(self.mem_ptr + self.pos), req_bytes)
			self.pos += req_bytes
			if req_bits != 0:
				memset(
					target_ptr + req_bytes,
					self.mem_ptr[self.pos] & ((2**req_bits) - 1),
					1
				)
				self.bitbuf = self.mem_ptr[self.pos] >> req_bits
				self.bitbuf_len = (8 - req_bits)
				self.pos += 1
		else:
			carry = self.bitbuf
			for i in range(req_bytes):
				memset(
					tmp_ptr,
					carry | (self.mem_ptr[self.pos + i] << self.bitbuf_len),
					1
				)
				tmp_ptr = target_ptr + 1
				carry = self.mem_ptr[self.pos + i] >> (8 - self.bitbuf_len)
			self.pos += req_bytes
			if req_bits == 0:
				self.bitbuf = carry
				# No other changes on bit level
			else:
				#Throw extra char at tmp_ptr, change bitbuf_len, update bitbuf
				if req_bits <= self.bitbuf_len:
					memset(tmp_ptr, self.bitbuf & ((2**req_bits) - 1), 1) # AND potentially unnecessary
					self.bitbuf >>= req_bits
					self.bitbuf_len -= req_bits
				else:
					# i. e. [0b[00000]100, 3], but i want 4 bits
					# so read the next byte into carry: i.e. 10010001
					# using fancy ops 2 lines below: [0000]1100
					# then save [0b[0]1001000, 7] as new bitbuf
					carry = self.mem_ptr[self.pos]
					memset(
						tmp_ptr, 
						self.bitbuf | ((carry << self.bitbuf_len) & ((2**req_bits) - 1)),
						1
					)
					# Ok because req_bits > bitbuf_len in this else block
					self.bitbuf = carry >> (req_bits - self.bitbuf_len)
					self.bitbuf_len = (8 - (req_bits - self.bitbuf_len))
					self.pos += 1

	cdef uint32_t dist_until_null(self):
		"""
		Returns the distance until the next nullbyte is encountered,
		that is the distance that - if it were added to self.pos would
		lead to a state where the previously read byte would be null.
		(>= 1)
		If EOB is hit, will cut off accordingly. MAY be 0 in only this case,
		also sets first bit of ERRORLEVEL to 1 then. 
		"""
		cdef uint32_t c_ln = 0
		cdef uint8_t cur_byte
		cdef uint8_t carry = self.bitbuf
		if self.bitbuf_len == 0:
			for _ in range(self.mem_len - self.pos):
				cur_byte = self.mem_ptr[self.pos + c_ln]
				c_ln += 1
				if cur_byte == 0x00:
					break
			else:
				self.ERRORLEVEL |= 0b1
		else:
			for _ in range(self.mem_len - self.pos):
				cur_byte = carry | (self.mem_ptr[self.pos + c_ln] << self.bitbuf_len)
				c_ln += 1
				if cur_byte == 0x00:
					break
				carry = (self.mem_ptr[self.pos + c_ln] >> (8 - self.bitbuf_len))
			else:
				self.ERRORLEVEL |= 0b1
		return c_ln

	cdef str get_next_utf8_str(self):
		"""
		Returns a python string with all chars up until the next null char
		converted to utf-8.
		May return an empty string on memory allocation failure or EOB.
		Will set second bit of ERRORLEVEL to 1 on alloc error.
		"""
		cdef uint32_t needed_len = self.dist_until_null()
		if needed_len == 0: #EOF
			return ""
		cdef uint8_t *tmp = <uint8_t *>malloc(needed_len)
		if tmp == NULL:
			self.ERRORLEVEL |= 0b10
			return ""
		self._read_raw(tmp, needed_len, 0)
		cdef str tmp_str = (tmp).decode("utf-8")
		free(tmp); tmp = NULL
		return tmp_str

	cdef str get_utf8_str(self, size_t req_len):
		"""
		Returns a python string with length req_len, decoded to utf-8.
		May return empty string on memory allocation failure.
		Will set second bit of ERRORLEVEL to 1 on alloc error.
		"""
		cdef uint8_t *tmp = <uint8_t *>malloc(req_len)
		if tmp == NULL:
			self.ERRORLEVEL |= 0b10
			return ""
		self._read_raw(tmp, req_len, 0)
		cdef str tmp_str = (tmp).decode("utf-8")
		free(tmp); tmp = NULL
		return tmp_str

	cdef uint64_t get_int(self, uint8_t req_bits):
		"""
		Returns requested amount of bits (0..64) as a 64 bit integer.
		Relatively unstable due to endian-ness, should be typecast.
		Returns 0 if more than 64 bits are requested.
		"""
		if req_bits > 64:
			return 0
		cdef uint64_t res = 0
		self._read_raw(&res, req_bits // 8, req_bits % 8)
		return res

	cdef uint32_t get_uint32(self):
		"""
		Returns the next 32 bits interpreted as an unsigned integer.
		"""
		cdef uint32_t i
		self._read_raw(&i, 4, 0)
		return i

	cdef float get_flt16(self):
		"""
		Returns the next 16 bits interpreted as a floating point number.
		"""
		cdef float f
		self._read_raw(&f, 2, 0)
		return f

	cdef double get_dbl32(self):
		"""
		Returns the next 32 bits interpreted as a floating point double.
		"""
		cdef double d
		self._read_raw(&d, 4, 0)
		return d
