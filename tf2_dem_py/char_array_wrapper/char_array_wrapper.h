
#ifndef CHAR_ARRAY_WRAPPER__H
#define CHAR_ARRAY_WRAPPER__H

#include <stdio.h>
#include <stdint.h>

#define ERR_BUFFER_TOO_SHORT    (1 << 0)
#define ERR_MEMORY_ALLOCATION   (1 << 1)
#define ERR_INIT_IO_READ        (1 << 2)
#define ERR_INIT_ALLOC          (1 << 3)
#define ERR_INIT_ODD_IO_RESULT  (1 << 4)

typedef struct CharArrayWrapper {
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
} CharArrayWrapper;

/* Create a new CharArrayWrapper and return a pointer to it.
 * May return a Nullpointer on CharArrayWrapper allocation failure.
 * Instantiated CharArrayWrapper may already be faulty, check ERRORLEVEL to be safe.
 */
CharArrayWrapper *CAW_from_file(FILE *fp, size_t initbytes);
CharArrayWrapper *CAW_from_buffer(void *buf, size_t length);
/* Deallocate a CharArrayWrapper's memory. */
void CAW_delete(CharArrayWrapper *caw);
/* Reads the bytes from the CharArrayWrapper's current position and bitbuffer offset
 * into the target pointer. It is in the caller's responsibility enough space is present
 * in the target pointer.
 */

void CAW_read_raw(CharArrayWrapper *caw, void *target_ptr, size_t req_bytes, uint8_t req_bits);
/* Skip requested amount of bits and bytes ahead. Sets first bit of ERRORLEVEL if buffer
 * is too short.
 */
void CAW_skip(CharArrayWrapper *caw, size_t bytes, uint8_t bits);
/* Returns the distance until the next nullbyte is encountered,
 * that is the amount of bytes that would have to be fetched to receive
 * a null-terminated string.
 * If end of the chararray is hit, ERRORLEVEL's first bit will be set to 1,
 * the returned value may be 0 only in this case, as it then is equal to
 * caw->mem_len - caw->bytepos. The block of this length will not end with null then.
 */
size_t CAW_dist_until_null(CharArrayWrapper *caw);
uint8_t CAW_remaining_bits(CharArrayWrapper *caw);
size_t CAW_remaining_bytes(CharArrayWrapper *caw);
/* Returns a pointer to an array of req_len chars.
 * Pointer has to be freed.
 * Will return Nullpointer and set second bit of ERRORLEVEL
 * to 1 on alloc error.
 */
uint8_t *CAW_get_chars(CharArrayWrapper *caw, size_t req_len);
/* Returns a pointer to the next nullterminated string.
 * Pointer has to be freed.
 * Will return a Nullpointer and set second bit of ERRORLEVEL to 1
 * on alloc error.
 */
uint8_t *CAW_get_nulltrm_str(CharArrayWrapper *caw);
/* Returns a var int from the stream. */
uint32_t CAW_get_var_int(CharArrayWrapper *caw);
/* Returns a bitcoord from the stream, according to an obscure valve specification.  */
float CAW_get_bit_coord(CharArrayWrapper *caw);
/* Returns the next 32 bits interpreted as a floating point number. */
float CAW_get_flt(CharArrayWrapper *caw);
/* Returns the next bit in an unsigned integer. */
uint8_t CAW_get_bit(CharArrayWrapper *caw);
/* Returns the next 8 bits interpreted as an unsigned integer. */
uint8_t CAW_get_uint8(CharArrayWrapper *caw);
/* Returns the next 16 bits interpreted as an unsigned integer. */
uint16_t CAW_get_uint16(CharArrayWrapper *caw);
/* Returns the next 32 bits interpreted as an unsigned integer. */
uint32_t CAW_get_uint32(CharArrayWrapper *caw);
/* Returns errorbyte from a CharArrayWrapper. */
uint8_t CAW_get_errorlevel(CharArrayWrapper *caw);

#endif
