
#ifndef CHAR_ARRAY_WRAPPER__H
#define CHAR_ARRAY_WRAPPER__H

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

#if (CHAR_BIT != 8)
#    error "What alien toaster are you compiling this on?"
#endif

extern const uint16_t CAW_ERR_BUFFER_TOO_SHORT;
extern const uint16_t CAW_ERR_MEMORY_ALLOCATION;
extern const uint16_t CAW_ERR_INIT_IO_READ;
extern const uint16_t CAW_ERR_INIT_ALLOC;
extern const uint16_t CAW_ERR_INIT_ODD_IO_RESULT;

typedef uint8_t CharArrayWrapper_err_t;

typedef struct CharArrayWrapper_s {
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
	//  0b 0 0 0 0 0 0 0 0
	//           | | | | \\ Buffer too short to perform requested read operation.
	//           | | | \\ Memory allocation failure.
	//           | | \\ Instantiation failure, read error.
	//           | \\ Instantiation failure, malloc error.
	//           \\ Instantiation failure, amount of bytes read differs from requested amount.
	CharArrayWrapper_err_t ERRORLEVEL;
	// Whether to free the CharArrayWrapper when its destroyed via CharArrayWrapper_destroy.
	uint8_t free_on_dealloc;

} CharArrayWrapper;

// Allocates space for a new CharArrayWrapper and initializes its fields.
CharArrayWrapper *CharArrayWrapper_new();

// Destroys a CharArrayWrapper by freeing its pointer and - if the free_on_dealloc
// attribute is set - its mem_ptr as well.
void CharArrayWrapper_destroy(CharArrayWrapper *self);

// Creates a new CharArrayWrapper and return a pointer to it.
// May return a Nullpointer on CharArrayWrapper allocation failure.
// Instantiated CharArrayWrapper may already be faulty, check ERRORLEVEL to be safe.
CharArrayWrapper *CharArrayWrapper_from_file(FILE *fp, size_t initbytes);

// Creates a CharArrayWrapper from an existing CharArrayWrapper.
// The new CharArrayWrapper's pos_byte will be 0, however may be offset
// on a bit level already, if the parent CharArrayWrapper was.
// As the new CharArrayWrapper works on the existing memory chunk
// of the old one, bit-level precision is not possible.
// DO NOT DEALLOCATE THE PARENT CAW BEFORE THE NEWLY CREATED ONE.
// Returns Null on allocation failure.
CharArrayWrapper *CharArrayWrapper_from_caw(CharArrayWrapper *self, size_t len);

// Creates a CharArrayWrapper on another one, however taking an amount
// of bits as input and performing some size requirement calculations.
// As size_t * 8 may exceed the capacity of size_t, this function is limited,
// but should work for smaller data blocks. The same byte-level restrictions
// as in from_caw apply.
// Returns Null on allocation failure.
CharArrayWrapper *CharArrayWrapper_from_caw_b(CharArrayWrapper *self, size_t bitlen);

// Verify enough data is left that can still be read, counting from
//     current bitbuffer length and position in char array.
// Returns:
//     0 if buffer is still large enough.
//     1 if the buffer is too short.
//     2 if more than 7 bits were requested.
uint8_t CharArrayWrapper__ver_buf_health(CharArrayWrapper *self, size_t req_bytes, uint8_t req_bits);

// Reads the bytes from the CharArrayWrapper's current position and bitbuffer offset
// into the target pointer. It is in the caller's responsibility enough space is present
// in the target pointer.
void CharArrayWrapper_read_raw(CharArrayWrapper *self, void *target_ptr, size_t req_bytes, uint8_t req_bits);

// Skip requested amount of bits and bytes ahead. Sets first bit of ERRORLEVEL if buffer
// is too short.
void CharArrayWrapper_skip(CharArrayWrapper *self, size_t bytes, uint8_t bits);

// Jump to a position in chararray. Sets first bit of ERRORLEVEL if buffer
// is too short. Has no effect if more than 7 bits are requested, so
// don't do that. Function is unsafe as it does not perform enough error checks.
// Note that specifying a byte X and a bit > 0 will cause the passed in
// CharArrayWrapper's bytepos field to be X + 1, as it always points to
// the index of the next untouched char.
void CharArrayWrapper_set_pos(CharArrayWrapper *self, size_t byte, uint8_t bit);

size_t CharArrayWrapper_get_pos_byte(CharArrayWrapper *self);

uint8_t CharArrayWrapper_get_pos_bit(CharArrayWrapper *self);

// Returns the distance until the next nullbyte is encountered,
// that is the amount of bytes that would have to be fetched to receive
// a null-terminated string.
// If end of the chararray is hit, ERRORLEVEL's first bit will be set to 1,
// the returned value may be 0 only in this case, as it then is equal to
// caw->mem_len - caw->bytepos. The block of this length will not end with null then.
size_t CharArrayWrapper_dist_until_null(CharArrayWrapper *self);

uint8_t CharArrayWrapper_remaining_bits(CharArrayWrapper *self);

size_t CharArrayWrapper_remaining_bytes(CharArrayWrapper *self);

// Returns a pointer to an array of req_len chars.
// Pointer has to be freed.
// Will return Nullpointer and set second bit of ERRORLEVEL
// to 1 on alloc error.
uint8_t *CharArrayWrapper_get_chars(CharArrayWrapper *self, size_t req_len);

// Returns a pointer to the next nullterminated string.
// Pointer has to be freed.
// Will return a Nullpointer and set second bit of ERRORLEVEL to 1
// on alloc error.
uint8_t *CharArrayWrapper_get_nulltrm_str(CharArrayWrapper *self);

// Returns a pointer to a nullterminated string capped at either the next
// nullbyte or - if none is encountered - up until len.
// The CAW will always be advanced by len bytes.
// Pointer has to be freed.
// Will return a Nullpointer and set ERRORLEVEL on errors.
uint8_t *CharArrayWrapper_get_chars_up_to_null(CharArrayWrapper *self, size_t len);

// Returns a var int from the stream.
uint32_t CharArrayWrapper_get_var_int(CharArrayWrapper *self);

// Returns a bitcoord from the stream, according to an obscure valve specification. 
float CharArrayWrapper_get_bit_coord(CharArrayWrapper *self);

// Returns the next 32 bits interpreted as a floating point number.
float CharArrayWrapper_get_flt(CharArrayWrapper *self);

// Returns the next bit in an unsigned integer.
uint8_t CharArrayWrapper_get_bit(CharArrayWrapper *self);

// Returns the next 8 bits interpreted as an unsigned integer.
uint8_t CharArrayWrapper_get_uint8(CharArrayWrapper *self);

// Returns the next 16 bits interpreted as an unsigned integer.
uint16_t CharArrayWrapper_get_uint16(CharArrayWrapper *self);

// Returns the next 16 bits interpreted as a signed integer.
int16_t CharArrayWrapper_get_int16(CharArrayWrapper *self);

// Returns the next 32 bits interpreted as an unsigned integer.
uint32_t CharArrayWrapper_get_uint32(CharArrayWrapper *self);

// Returns the next 32 bits interpreted as a signed integer.
int32_t CharArrayWrapper_get_int32(CharArrayWrapper *self);


#endif
