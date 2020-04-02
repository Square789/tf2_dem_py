
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "char_array_wrapper.h"

CharArrayWrapper *CAW_from_file(FILE *fp, size_t initbytes) {
	CharArrayWrapper *caw_ptr = (CharArrayWrapper *)malloc(sizeof(CharArrayWrapper));
	if (caw_ptr == NULL) {
		return NULL;
	}
	caw_ptr->mem_len = initbytes;
	caw_ptr->bitbuf = 0x0;
	caw_ptr->bitbuf_len = 0;
	caw_ptr->bytepos = 0;
	caw_ptr->ERRORLEVEL = 0;
	caw_ptr->free_on_dealloc = 1;

	caw_ptr->mem_ptr = (uint8_t *)malloc(initbytes);
	if (caw_ptr->mem_ptr == NULL) {
		caw_ptr->ERRORLEVEL |= ERR_INIT_ALLOC;
	}

	size_t read_res = fread(caw_ptr->mem_ptr, sizeof(uint8_t), initbytes, fp);
	if (ferror(fp)) {
		caw_ptr->ERRORLEVEL |= ERR_INIT_IO_READ;
	}
	if (read_res != initbytes) {
		caw_ptr->ERRORLEVEL |= ERR_INIT_ODD_IO_RESULT;
	}
	return caw_ptr;
}

// CharArrayWrapper *CAW_from_buffer(void *buf, size_t length, uint8_t free_on_dealloc) {
// 	CharArrayWrapper *caw_ptr = (CharArrayWrapper *)malloc(sizeof(CharArrayWrapper));
// 	if (caw_ptr == NULL) {
// 		return NULL;
// 	}
// 	caw_ptr->mem_len = length;
// 	caw_ptr->bitbuf = 0x0;
// 	caw_ptr->bitbuf_len = 0;
// 	caw_ptr->bytepos = 0;
// 	caw_ptr->ERRORLEVEL = 0;
// 	caw_ptr->free_on_dealloc = free_on_dealloc;

// 	caw_ptr->mem_ptr = (uint8_t *)buf;

// 	return caw_ptr;
// }

CharArrayWrapper *CAW_from_caw(CharArrayWrapper *caw, size_t len) {
	CharArrayWrapper *new_caw = (CharArrayWrapper *)malloc(sizeof(CharArrayWrapper));
	if (new_caw == NULL) {
		return NULL;
	}
	new_caw->mem_len = len;
	new_caw->mem_ptr = caw->mem_ptr + CAW_get_pos_byte(caw);
	new_caw->bytepos = 0;
	new_caw->bitbuf = 0;
	new_caw->bitbuf_len = 0;
	new_caw->ERRORLEVEL = 0;
	new_caw->free_on_dealloc = 0;

	CAW_skip(new_caw, 0, CAW_get_pos_bit(caw));
	return new_caw;
}


CharArrayWrapper *CAW_from_caw_b(CharArrayWrapper *caw, uint64_t bitlen) {
	size_t req_bytelen = 0;
	bitlen += CAW_get_pos_bit(caw);
	if (bitlen % 8 == 0) {
		req_bytelen = (bitlen / 8);
	} else {
		req_bytelen = (bitlen / 8) + 1;
	}
	CharArrayWrapper *user_message_caw = CAW_from_caw(caw, req_bytelen);
	bitlen -= CAW_get_pos_bit(caw); // Remove adjustment bits again
	CAW_skip(caw, bitlen / 8, bitlen % 8);
	return user_message_caw;
}

void CAW_delete(CharArrayWrapper *caw) {
	if (caw->free_on_dealloc != 0) {
		free(caw->mem_ptr);
		caw->mem_ptr = NULL;
	}
	free(caw);
	caw = NULL;
}

/*
 * Verify enough data is left that can still be read, counting from
 *     current bitbuffer length and position in char array.
 * Returns:
 *     0 if buffer is still large enough.
 *     1 if the buffer is too short.
 *     2 if more than 7 bits were requested.
 */
inline uint8_t CAW__ver_buf_health(CharArrayWrapper *caw, size_t req_bytes, uint8_t req_bits) {
	if (req_bits > 7)
		return 2;
	// Temp var in case bitbuf overflows
	uint8_t tmp;
	if (req_bits > caw->bitbuf_len) { tmp = 1; } else { tmp = 0; }
	if ((caw->mem_len - caw->bytepos) < (req_bytes + tmp))
		return 1;
	return 0;
}

void CAW_read_raw(CharArrayWrapper *caw, void *target_ptr, size_t req_bytes, uint8_t req_bits) {
	uint8_t carry; // temporary bit level storage
	size_t i; // Loop variable
	if (CAW__ver_buf_health(caw, req_bytes, req_bits) != 0) {
		caw->ERRORLEVEL |= ERR_BUFFER_TOO_SHORT;
		return;
	}
	if (caw->bitbuf_len == 0) { // No bit offset; Should work faster
		memcpy(target_ptr, (void *)(caw->mem_ptr + caw->bytepos), req_bytes);
		caw->bytepos += req_bytes;
		if (req_bits != 0) {
			memset(
				target_ptr + req_bytes,
				caw->mem_ptr[caw->bytepos] & ((1 << req_bits) - 1),
				1
			);
			caw->bitbuf = caw->mem_ptr[caw->bytepos] >> req_bits;
			caw->bitbuf_len = (8 - req_bits);
			caw->bytepos += 1;
		}
	} else { // Offset on bitlevel, shifting required
		carry = caw->bitbuf;
		for (i = 0; i < req_bytes; i++) {
			memset(
				target_ptr + i,
				carry | (caw->mem_ptr[caw->bytepos + i] << caw->bitbuf_len),
				1
			);
			carry = caw->mem_ptr[caw->bytepos + i] >> (8 - caw->bitbuf_len);
		}
		caw->bytepos += req_bytes;
		caw->bitbuf = carry;
		if (req_bits != 0) { // Otherwise done
			// Throw extra char at tmp_ptr, change bitbuf_len, update bitbuf
			if (req_bits <= caw->bitbuf_len) { // Drain bitbuf of required bits
				memset(target_ptr + req_bytes, caw->bitbuf & ((1 << req_bits) - 1), 1);
				caw->bitbuf >>= req_bits;
				caw->bitbuf_len -= req_bits;
			} else { // "Read" another byte from chararray
				// i. e. [0b[00000]100, 3], but i want 4 bits
				// so read the next byte into carry: i.e. 10010001
				// using fancy ops 2 lines below: [0000]1100
				// then save [0b[0]1001000, 7] as new bitbuf
				carry = caw->mem_ptr[caw->bytepos];
				memset(
					target_ptr + req_bytes,
					caw->bitbuf | ((carry << caw->bitbuf_len) & ((1 << req_bits) - 1)),
					1
				);
				// Ok because req_bits > bitbuf_len in this else block
				caw->bitbuf = carry >> (req_bits - caw->bitbuf_len);
				caw->bitbuf_len = (8 - (req_bits - caw->bitbuf_len));
				caw->bytepos += 1;
			}
		}
	}
}

void CAW_skip(CharArrayWrapper *caw, size_t bytes, uint8_t bits) {
	if (CAW__ver_buf_health(caw, bytes, bits) != 0) {
		caw->ERRORLEVEL |= ERR_BUFFER_TOO_SHORT;
		return;
	}

	caw->bytepos += bytes; // Skip bytes

	if (caw->bitbuf_len != 0) { // Adjust bitbuffer (length remains)
		caw->bitbuf = (caw->mem_ptr[caw->bytepos - 1]) >> (8 - caw->bitbuf_len);
	}

	if (bits != 0) {
		if (bits <= caw->bitbuf_len) { // Shorten bitbuf
			caw->bitbuf >>= bits;
			caw->bitbuf_len -= bits;
		} else { // "Read" next byte from chararray
			caw->bitbuf = (caw->mem_ptr[caw->bytepos] >> (bits - caw->bitbuf_len));
			caw->bitbuf_len = (8 - (bits - caw->bitbuf_len));
			caw->bytepos += 1;
		}
	}
}

size_t CAW_get_pos_byte(CharArrayWrapper *caw) {
	if (caw->bitbuf_len == 0) { // Next unread byte is basically position
		return caw->bytepos;
	} else { // Untouched data in bitbuffer -> pos is still in current byte
		return caw->bytepos - 1;
	}
}

uint8_t CAW_get_pos_bit(CharArrayWrapper *caw) {
	if (caw->bitbuf_len == 0) {
		return 0;
	} else {
		return (8 - caw->bitbuf_len);
	}
	// 0 0 1 0 1 1|0 1
	// -----6-----|-2-
	// -> pos = 8 - 6 = 2
}

void CAW_set_pos(CharArrayWrapper *caw, size_t byte, uint8_t bit) {
	if (bit > 7) { return; }
	if (byte > caw->mem_len) {
		caw->ERRORLEVEL |= ERR_BUFFER_TOO_SHORT;
		return;
	}
	caw->bytepos = byte;
	caw->bitbuf = 0;
	caw->bitbuf_len = 0;

	CAW_skip(caw, 0, bit);
}

size_t CAW_dist_until_null(CharArrayWrapper *caw) {
	size_t c_ln = 0;
	uint8_t cur_byte;
	size_t maxdist = caw->mem_len - caw->bytepos;
	size_t i = 0;
	uint8_t carry = caw->bitbuf;
	if (caw->bitbuf_len == 0) {
		for (i = 0; i < maxdist; i++) {
			cur_byte = caw->mem_ptr[caw->bytepos + i];
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
		}
	} else {
		for (i = 0; i < maxdist; i++) {
			// Current byte is bitbuffer ORed with the next unread byte shifted to fill rest of bitbuffer
			cur_byte = carry | ((caw->mem_ptr[caw->bytepos + i]) << (caw->bitbuf_len));
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
			carry = ((caw->mem_ptr[caw->bytepos + i]) >> (8 - caw->bitbuf_len));
		}
	}
	caw->ERRORLEVEL |= ERR_BUFFER_TOO_SHORT;
dist_until_null_break:
	return c_ln;
}

uint8_t CAW_remaining_bits(CharArrayWrapper *caw) {
	return caw->bitbuf_len;
}

size_t CAW_remaining_bytes(CharArrayWrapper *caw) {
	return (caw->mem_len - caw->bytepos);
}

uint8_t *CAW_get_chars(CharArrayWrapper *caw, size_t req_len) {
	uint8_t *ptr = (uint8_t *)malloc(req_len);
	if (ptr == NULL) {
		caw->ERRORLEVEL |= ERR_MEMORY_ALLOCATION;
		return ptr;
	}
	CAW_read_raw(caw, ptr, req_len, 0);
	return ptr;
}

uint8_t *CAW_get_nulltrm_str(CharArrayWrapper *caw) {
	uint8_t *res_ptr;
	size_t ntstr_ln = CAW_dist_until_null(caw);
	if (caw->ERRORLEVEL & ERR_BUFFER_TOO_SHORT) { // set by method above
		return NULL;
	}
	res_ptr = malloc(ntstr_ln);
	if (res_ptr == NULL) {
		caw->ERRORLEVEL |= ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	CAW_read_raw(caw, res_ptr, ntstr_ln, 0);
	return res_ptr;
}

uint32_t CAW_get_var_int(CharArrayWrapper *caw) {
	uint32_t res = 0;
	uint8_t i = 0;
	uint8_t read;
	for (i = 0; i < 35; i += 7) {
		read = CAW_get_uint8(caw);
		res |= ((read & 0x7F) << i);
		if ((read >> 7) == 0) {
			break;
		}
	}
	return res;
}

float CAW_get_bit_coord(CharArrayWrapper *caw) {
	uint8_t has_dec, has_frac;
	uint8_t frac_val = 0;
	int8_t sign;
	uint16_t decimal_val = 0;
	has_dec = CAW_get_bit(caw);
	has_frac = CAW_get_bit(caw);
	if (!(has_dec | has_frac)) {
		return 0.0f;
	}
	if (CAW_get_bit(caw) == 1) {
		sign = 1;
	} else {
		sign = -1;
	}
	if (has_dec == 1) {
		CAW_read_raw(caw, &decimal_val, 1, 6);
	}
	if (has_frac == 1) {
		CAW_read_raw(caw, &frac_val, 0, 5);
	}
	return sign * (decimal_val + (frac_val * 0.03125f));
}

float CAW_get_flt(CharArrayWrapper *caw) {
	float f;
	CAW_read_raw(caw, &f, 4, 0);
	return f;
}

uint8_t CAW_get_bit(CharArrayWrapper *caw) {
	uint8_t i;
	CAW_read_raw(caw, &i, 0, 1);
	return i;
}

uint8_t CAW_get_uint8(CharArrayWrapper *caw) {
	uint8_t i;
	CAW_read_raw(caw, &i, 1, 0);
	return i;
}

uint16_t CAW_get_uint16(CharArrayWrapper *caw) {
	uint16_t i;
	CAW_read_raw(caw, &i, 2, 0);
	return i;
}

uint32_t CAW_get_uint32(CharArrayWrapper *caw) {
	uint32_t i;
	CAW_read_raw(caw, &i, 4, 0);
	return i;
}

uint8_t CAW_get_errorlevel(CharArrayWrapper *caw) {
	return caw->ERRORLEVEL;
}
