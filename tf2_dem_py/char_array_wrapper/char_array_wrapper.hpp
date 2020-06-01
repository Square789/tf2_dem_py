
#ifndef CHAR_ARRAY_WRAPPER__HPP
#define CHAR_ARRAY_WRAPPER__HPP

#include <stdio.h>
#include <stdint.h>

static const uint16_t CAW_ERR_BUFFER_TOO_SHORT   = (1 << 0);
static const uint16_t CAW_ERR_MEMORY_ALLOCATION  = (1 << 1);
static const uint16_t CAW_ERR_INIT_IO_READ       = (1 << 2);
static const uint16_t CAW_ERR_INIT_ALLOC         = (1 << 3);
static const uint16_t CAW_ERR_INIT_ODD_IO_RESULT = (1 << 4);

class CharArrayWrapper {
public:
	// Pointer to the memory block the CharArrayWrapper works on.
	uint8_t *mem_ptr;
	// Length of the CharArrayWrapper's memory block.
	size_t mem_len;
	// Bitbuffer to store intermediate bit values.
	uint8_t bitbuf;
	// Virtual length of the bitbuffer.
	uint8_t bitbuf_len;
	// Byteposition in the CharArrayWrapper; mem_ptr[bytepos] is the next unread byte.
	size_t bytepos;
	/*	0b 0 0 0 0 0 0 0 0
	 *           | | | | \\ Buffer too short to perform requested read operation.
	 *           | | | \\ Memory allocation failure.
	 *           | | \\ Instantiation failure, read error.
	 *           | \\ Instantiation failure, malloc error.
	 *           \\ Instantiation failure, amount of bytes read differs from requested amount.
	 */
	uint8_t ERRORLEVEL;
	uint8_t free_on_dealloc;

	CharArrayWrapper(uint8_t *mem_ptr, size_t mem_len);
	~CharArrayWrapper();

	/* Create a CharArrayWrapper from an existing CharArrayWrapper.
	* The new CharArrayWrapper's pos_byte will be 0, however may be offset
	* on a bit level already, if the parent CharArrayWrapper was.
	* As the new CharArrayWrapper works on the existing memory chunk
	* of the old one, bit-level precision is not possible.
	* DO NOT DEALLOCATE THE PARENT CAW BEFORE THE NEWLY CREATED ONE.
	* Returns Null on allocation failure.
	*/
	CharArrayWrapper *caw_from_caw(size_t len);
	/* Create a CharArrayWrapper on another one, however taking an amount
	* of bits as input and performing some size requirement calculations.
	* As size_t * 8 may exceed the capacity of uint64_t, this function is limited,
	* but should work for smaller data blocks. The same byte-level restrictions
	* as in from_caw apply.
	* Returns Null on allocation failure.
	*/
	CharArrayWrapper *caw_from_caw_b(uint64_t bitlen);

protected: 
	/*
	* Verify enough data is left that can still be read, counting from
	*     current bitbuffer length and position in char array.
	* Returns:
	*     0 if buffer is still large enough.
	*     1 if the buffer is too short.
	*     2 if more than 7 bits were requested.
	*/
	uint8_t _ver_buf_health(size_t req_bytes, uint8_t req_bits);

public:
	/* Reads the bytes from the CharArrayWrapper's current position and bitbuffer offset
	* into the target pointer. It is in the caller's responsibility enough space is present
	* in the target pointer.
	*/
	void read_raw(void *target_ptr, size_t req_bytes, uint8_t req_bits);
	/* Skip requested amount of bits and bytes ahead. Sets first bit of ERRORLEVEL if buffer
	* is too short.
	*/
	void skip(size_t bytes, uint8_t bits);
	/* Jump to a position in chararray. Sets first bit of ERRORLEVEL if buffer
	* is too short. Has no effect if more than 7 bits are requested, so
	* don't do that. Function is unsafe as it does not perform enough error checks.
	* Note that specifying a byte X and a bit > 0 will cause the passed in
	* CharArrayWrapper's bytepos field to be X + 1, as it always points to
	* the index of the next untouched char.
	*/
	void set_pos(size_t byte, uint8_t bit);
	size_t get_pos_byte();
	uint8_t get_pos_bit();
	/* Returns the distance until the next nullbyte is encountered,
	* that is the amount of bytes that would have to be fetched to receive
	* a null-terminated string.
	* If end of the chararray is hit, ERRORLEVEL's first bit will be set to 1,
	* the returned value may be 0 only in this case, as it then is equal to
	* caw->mem_len - caw->bytepos. The block of this length will not end with null then.
	*/
	size_t dist_until_null();
	uint8_t remaining_bits();
	size_t remaining_bytes();
	/* Returns a pointer to an array of req_len chars.
	* Pointer has to be freed.
	* Will return Nullpointer and set second bit of ERRORLEVEL
	* to 1 on alloc error.
	*/
	char *get_chars(size_t req_len);
	/* Returns a pointer to the next nullterminated string.
	* Pointer has to be freed.
	* Will return a Nullpointer and set second bit of ERRORLEVEL to 1
	* on alloc error.
	*/
	char *get_nulltrm_str();
	/* Returns a var int from the stream. */
	uint32_t get_var_int();
	/* Returns a bitcoord from the stream, according to an obscure valve specification.  */
	float get_bit_coord();
	/* Returns the next 32 bits interpreted as a floating point number. */
	float get_flt();
	/* Returns the next bit in an unsigned integer. */
	uint8_t get_bit();
	/* Returns the next 8 bits interpreted as an unsigned integer. */
	uint8_t get_uint8();
	/* Returns the next 16 bits interpreted as an unsigned integer. */
	uint16_t get_uint16();
	/* Returns the next 32 bits interpreted as an unsigned integer. */
	uint32_t get_uint32();
	/* Returns error number from a CharArrayWrapper. Used for cython integration, as -> is not supported. */
	uint8_t get_errorlevel();
};

/* Create a new CharArrayWrapper and return a pointer to it.
* May return a Nullpointer on CharArrayWrapper allocation failure.
* Instantiated CharArrayWrapper may already be faulty, check ERRORLEVEL to be safe.
*/
CharArrayWrapper *CAW_from_file(FILE *fp, size_t initbytes);

#endif
