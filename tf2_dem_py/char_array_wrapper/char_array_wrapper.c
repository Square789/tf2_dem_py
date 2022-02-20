
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.h"

const uint16_t CAW_ERR_BUFFER_TOO_SHORT   = (1 << 0);
const uint16_t CAW_ERR_MEMORY_ALLOCATION  = (1 << 1);
const uint16_t CAW_ERR_INIT_IO_READ       = (1 << 2);
const uint16_t CAW_ERR_INIT_ALLOC         = (1 << 3);
const uint16_t CAW_ERR_INIT_ODD_IO_RESULT = (1 << 4);


CharArrayWrapper *CharArrayWrapper_from_file(FILE *fp, size_t initbytes) {
	size_t read_res;
	CharArrayWrapper *new_caw = CharArrayWrapper_new();
	if (new_caw == NULL) {
		return NULL;
	}
	new_caw->mem_len = initbytes;
	new_caw->free_on_dealloc = 1;

	new_caw->mem_ptr = (uint8_t *)malloc(initbytes);
	if (new_caw->mem_ptr == NULL) {
		new_caw->ERRORLEVEL |= CAW_ERR_INIT_ALLOC;
		return new_caw;
	}

	read_res = fread(new_caw->mem_ptr, sizeof(uint8_t), initbytes, fp);
	if (ferror(fp)) {
		new_caw->ERRORLEVEL |= CAW_ERR_INIT_IO_READ;
	}
	if (read_res != initbytes) {
		new_caw->ERRORLEVEL |= CAW_ERR_INIT_ODD_IO_RESULT;
	}
	return new_caw;
}

CharArrayWrapper *CharArrayWrapper_from_caw(CharArrayWrapper *self, size_t len) {
	CharArrayWrapper *new_caw = CharArrayWrapper_new();
	if (new_caw == NULL) {
		return NULL;
	}
	new_caw->mem_ptr = self->mem_ptr + CharArrayWrapper_get_pos_byte(self);
	new_caw->mem_len = len;
	new_caw->free_on_dealloc = 0;

	CharArrayWrapper_skip(new_caw, 0, CharArrayWrapper_get_pos_bit(self));
	return new_caw;
}

CharArrayWrapper *CharArrayWrapper_from_caw_b(CharArrayWrapper *self, size_t bitlen) {
	size_t req_bytelen;
	CharArrayWrapper *new_caw;
	bitlen += CharArrayWrapper_get_pos_bit(self);
	if (bitlen % 8 == 0) {
		req_bytelen = (bitlen / 8);
	} else {
		req_bytelen = (bitlen / 8) + 1;
	}
	new_caw = CharArrayWrapper_from_caw(self, req_bytelen);
	if (new_caw == NULL) {
		return NULL;
	}
	bitlen -= CharArrayWrapper_get_pos_bit(self); // Remove adjustment bits again
	CharArrayWrapper_skip(self, bitlen / 8, bitlen % 8);
	return new_caw;
}

CharArrayWrapper *CharArrayWrapper_new() {
	CharArrayWrapper *ptr = (CharArrayWrapper *)malloc(sizeof(CharArrayWrapper));
	if (ptr == NULL) {
		return NULL;
	}
	ptr->mem_ptr = NULL;
	ptr->mem_len = 0;
	ptr->bitbuf = 0;
	ptr->bitbuf_len = 0;
	ptr->bytepos = 0;
	ptr->ERRORLEVEL = 0;
	ptr->free_on_dealloc = 1;

	return ptr;
}

void CharArrayWrapper_destroy(CharArrayWrapper *self) {
	if (self->free_on_dealloc != 0) {
		free(self->mem_ptr);
		self->mem_ptr = NULL;
	}
	free(self);
}

uint8_t CharArrayWrapper__ver_buf_health(CharArrayWrapper *self, size_t req_bytes, uint8_t req_bits) {
		if (req_bits > 7) {
			return 2;
		}
		// If bitbuf overflows, add 1 to the compared remaining length in bytes
		if ((self->mem_len - self->bytepos) < (req_bytes + (req_bits > self->bitbuf_len))) {
			return 1;
		}
		return 0;
	}

void CharArrayWrapper_read_raw(CharArrayWrapper *self, void *target_ptr_, size_t req_bytes, uint8_t req_bits) {
	uint8_t carry; // temporary bit level storage
	size_t i; // Loop variable

	uint8_t *target_ptr = (uint8_t *)target_ptr_;

	if (CharArrayWrapper__ver_buf_health(self, req_bytes, req_bits) != 0) {
		self->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}
	if (self->bitbuf_len == 0) { // No bit offset; Should work faster
		memcpy(target_ptr, (void *)(self->mem_ptr + self->bytepos), req_bytes);
		self->bytepos += req_bytes;
		if (req_bits != 0) {
			*(target_ptr + req_bytes) = self->mem_ptr[self->bytepos] & ((1 << req_bits) - 1);
			self->bitbuf = self->mem_ptr[self->bytepos] >> req_bits;
			self->bitbuf_len = (8 - req_bits);
			self->bytepos += 1;
		}
	} else { // Offset on bitlevel, shifting required
		carry = self->bitbuf;
		for (i = 0; i < req_bytes; i++) {
			*(target_ptr + i) = carry | (self->mem_ptr[self->bytepos + i] << self->bitbuf_len);
			carry = self->mem_ptr[self->bytepos + i] >> (8 - self->bitbuf_len);
		}
		self->bytepos += req_bytes;
		self->bitbuf = carry;
		if (req_bits != 0) { // Otherwise done
			// Throw extra char at target_ptr, change bitbuf_len, update bitbuf
			if (req_bits <= self->bitbuf_len) {
				// Drain bitbuf of required bits
				*(target_ptr + req_bytes) = self->bitbuf & ((1 << req_bits) - 1);
				self->bitbuf >>= req_bits;
				self->bitbuf_len -= req_bits;
			} else {
				// "Read" another byte from chararray i. e. [0b[00000]100, 3],
				// but i want 4 bits so read the next byte into carry:
				// i.e. 10010001 using fancy ops 2 lines below: [0000]1100
				// then save [0b[0]1001000, 7] as new bitbuf
				carry = self->mem_ptr[self->bytepos];
				*(target_ptr + req_bytes) = self->bitbuf | ((carry << self->bitbuf_len) & ((1 << req_bits) - 1));
				// Ok because req_bits > bitbuf_len in this else block
				self->bitbuf = carry >> (req_bits - self->bitbuf_len);
				self->bitbuf_len = (8 - (req_bits - self->bitbuf_len));
				self->bytepos += 1;
			}
		}
	}
}

void CharArrayWrapper_skip(CharArrayWrapper *self, size_t bytes, uint8_t bits) {
	if (CharArrayWrapper__ver_buf_health(self, bytes, bits) != 0) {
		self->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}

	self->bytepos += bytes; // Skip bytes

	if (self->bitbuf_len != 0) { // Adjust bitbuffer (length remains)
		self->bitbuf = (self->mem_ptr[self->bytepos - 1]) >> (8 - self->bitbuf_len);
	}

	if (bits != 0) {
		if (bits <= self->bitbuf_len) { // Drain bitbuf
			self->bitbuf >>= bits;
			self->bitbuf_len -= bits;
		} else { // "Read" next byte from chararray
			self->bitbuf = (self->mem_ptr[self->bytepos] >> (bits - self->bitbuf_len));
			self->bitbuf_len = (8 - (bits - self->bitbuf_len));
			self->bytepos += 1;
		}
	}
}

size_t CharArrayWrapper_get_pos_byte(CharArrayWrapper *self) {
	if (self->bitbuf_len == 0) { // Next unread byte is basically position
		return self->bytepos;
	} else { // Untouched data in bitbuffer -> pos is still in current byte
		return self->bytepos - 1;
	}
}

uint8_t CharArrayWrapper_get_pos_bit(CharArrayWrapper *self) {
	if (self->bitbuf_len == 0) {
		return 0;
	} else {
		return (8 - self->bitbuf_len);
	}
	// 0 0 1 0 1 1|0 1
	// -----6-----|-2-
	// -> pos = 8 - 6 = 2
}

void CharArrayWrapper_set_pos(CharArrayWrapper *self, size_t byte, uint8_t bit) {
	if (bit > 7) {
		return;
	}
	if (byte > self->mem_len) {
		self->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}
	self->bytepos = byte;
	self->bitbuf = 0;
	self->bitbuf_len = 0;

	CharArrayWrapper_skip(self, 0, bit);
}

size_t CharArrayWrapper_dist_until_null(CharArrayWrapper *self) {
	size_t c_ln = 0;
	uint8_t cur_byte;
	size_t maxdist = self->mem_len - self->bytepos;
	size_t i = 0;
	uint8_t carry = self->bitbuf;
	if (self->bitbuf_len == 0) {
		for (i = 0; i < maxdist; i++) {
			cur_byte = self->mem_ptr[self->bytepos + i];
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
		}
	} else {
		for (i = 0; i < maxdist; i++) {
			// Current byte is bitbuffer ORed with the next unread byte shifted to fill rest of bitbuffer
			cur_byte = carry | ((self->mem_ptr[self->bytepos + i]) << (self->bitbuf_len));
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
			carry = ((self->mem_ptr[self->bytepos + i]) >> (8 - self->bitbuf_len));
		}
	}
	self->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
dist_until_null_break:
	return c_ln;
}

uint8_t CharArrayWrapper_remaining_bits(CharArrayWrapper *self) {
	return self->bitbuf_len;
}

size_t CharArrayWrapper_remaining_bytes(CharArrayWrapper *self) {
	return (self->mem_len - self->bytepos);
}

uint8_t *CharArrayWrapper_get_chars(CharArrayWrapper *self, size_t req_len) {
	uint8_t *res_ptr = (uint8_t *)malloc(req_len + 1);
	if (res_ptr == NULL) {
		self->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	CharArrayWrapper_read_raw(self, res_ptr, req_len, 0);
	res_ptr[req_len] = 0x00;
	return res_ptr;
}

uint8_t *CharArrayWrapper_get_nulltrm_str(CharArrayWrapper *self) {
	uint8_t *res_ptr;
	size_t ntstr_ln = CharArrayWrapper_dist_until_null(self);
	if (self->ERRORLEVEL & CAW_ERR_BUFFER_TOO_SHORT) { // set by method above
		return NULL;
	}
	res_ptr = (uint8_t *)malloc(ntstr_ln);
	if (res_ptr == NULL) {
		self->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	CharArrayWrapper_read_raw(self, res_ptr, ntstr_ln, 0);
	return res_ptr;
}

uint8_t *CharArrayWrapper_get_chars_up_to_null(CharArrayWrapper *self, size_t len) {
	uint8_t *res_ptr;
	size_t ntstr_ln = CharArrayWrapper_dist_until_null(self);
	if (self->ERRORLEVEL & CAW_ERR_BUFFER_TOO_SHORT) { // set by method above
		return NULL;
	}
	if (ntstr_ln < len) { // Pick the shorter one
		res_ptr = (uint8_t *)malloc(ntstr_ln);
		if (res_ptr == NULL) {
			self->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
			return NULL;
		}
		CharArrayWrapper_read_raw(self, res_ptr, ntstr_ln, 0);
		CharArrayWrapper_skip(self, len - ntstr_ln, 0);
	} else {
		res_ptr = (uint8_t *)malloc(len + 1);
		if (res_ptr == NULL) {
			self->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
			return NULL;
		}
		CharArrayWrapper_read_raw(self, res_ptr, len, 0);
		res_ptr[len] = 0x00;
	}
	return res_ptr;
}

uint32_t CharArrayWrapper_get_var_int(CharArrayWrapper *self) {
	uint32_t res = 0;
	uint8_t i = 0;
	uint8_t read;
	for (i = 0; i < 35; i += 7) {
		read = CharArrayWrapper_get_uint8(self);
		res |= ((read & 0x7F) << i);
		if ((read >> 7) == 0) {
			break;
		}
	}
	return res;
}

float CharArrayWrapper_get_bit_coord(CharArrayWrapper *self) {
	uint8_t has_dec, has_frac;
	uint8_t frac_val = 0;
	int8_t sign;
	uint16_t decimal_val = 0;
	has_dec = CharArrayWrapper_get_bit(self);
	has_frac = CharArrayWrapper_get_bit(self);
	if (!(has_dec | has_frac)) {
		return 0.0f;
	}
	if (CharArrayWrapper_get_bit(self) == 1) {
		sign = 1;
	} else {
		sign = -1;
	}
	if (has_dec == 1) {
		CharArrayWrapper_read_raw(self, &decimal_val, 1, 6);
	}
	if (has_frac == 1) {
		CharArrayWrapper_read_raw(self, &frac_val, 0, 5);
	}
	return sign * (decimal_val + (frac_val * 0.03125f));
}

float CharArrayWrapper_get_flt(CharArrayWrapper *self) {
	float f;
	CharArrayWrapper_read_raw(self, &f, 4, 0);
	return f;
}

uint8_t CharArrayWrapper_get_bit(CharArrayWrapper *self) {
	uint8_t i;
	CharArrayWrapper_read_raw(self, &i, 0, 1);
	return i;
}

uint8_t CharArrayWrapper_get_uint8(CharArrayWrapper *self) {
	uint8_t i;
	CharArrayWrapper_read_raw(self, &i, 1, 0);
	return i;
}

uint16_t CharArrayWrapper_get_uint16(CharArrayWrapper *self) {
	uint16_t i;
	CharArrayWrapper_read_raw(self, &i, 2, 0);
	return i;
}

int16_t CharArrayWrapper_get_int16(CharArrayWrapper *self) {
	int16_t i;
	CharArrayWrapper_read_raw(self, &i, 2, 0);
	return i;
}

uint32_t CharArrayWrapper_get_uint32(CharArrayWrapper *self) {
	uint32_t i;
	CharArrayWrapper_read_raw(self, &i, 4, 0);
	return i;
}

int32_t CharArrayWrapper_get_int32(CharArrayWrapper *self) {
	int32_t i;
	CharArrayWrapper_read_raw(self, &i, 4, 0);
	return i;
}
