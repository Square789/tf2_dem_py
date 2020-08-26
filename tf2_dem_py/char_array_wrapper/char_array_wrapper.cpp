
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "tf2_dem_py/char_array_wrapper/char_array_wrapper.hpp"

const uint16_t CAW_ERR_BUFFER_TOO_SHORT   = (1 << 0);
const uint16_t CAW_ERR_MEMORY_ALLOCATION  = (1 << 1);
const uint16_t CAW_ERR_INIT_IO_READ       = (1 << 2);
const uint16_t CAW_ERR_INIT_ALLOC         = (1 << 3);
const uint16_t CAW_ERR_INIT_ODD_IO_RESULT = (1 << 4);

CharArrayWrapper *caw_from_file(FILE *fp, size_t initbytes) {
	CharArrayWrapper *caw_ptr = new CharArrayWrapper(NULL, 0);
	size_t read_res;
	if (caw_ptr == NULL) {
		return NULL;
	}
	caw_ptr->mem_len = initbytes;
	caw_ptr->free_on_dealloc = 1;

	caw_ptr->mem_ptr = (uint8_t *)malloc(initbytes);
	if (caw_ptr->mem_ptr == NULL) {
		caw_ptr->ERRORLEVEL |= CAW_ERR_INIT_ALLOC;
		return caw_ptr;
	}

	read_res = fread(caw_ptr->mem_ptr, sizeof(uint8_t), initbytes, fp);
	if (ferror(fp)) {
		caw_ptr->ERRORLEVEL |= CAW_ERR_INIT_IO_READ;
	}
	//printf("[caw init]: read %d bytes: <%s>\n", read_res, caw_ptr->mem_ptr);
	if (read_res != initbytes) {
		caw_ptr->ERRORLEVEL |= CAW_ERR_INIT_ODD_IO_RESULT;
	}
	return caw_ptr;
}

CharArrayWrapper *CharArrayWrapper::caw_from_caw(size_t len) {
	CharArrayWrapper *new_caw = new CharArrayWrapper(NULL, 0);
	if (new_caw == NULL) {
		return NULL;
	}
	new_caw->mem_len = len;
	new_caw->mem_ptr = this->mem_ptr + this->get_pos_byte();
	new_caw->bytepos = 0;
	new_caw->bitbuf = 0;
	new_caw->bitbuf_len = 0;
	new_caw->ERRORLEVEL = 0;
	new_caw->free_on_dealloc = 0;

	new_caw->skip(0, this->get_pos_bit());
	return new_caw;
}


CharArrayWrapper *CharArrayWrapper::caw_from_caw_b(uint64_t bitlen) {
	size_t req_bytelen = 0;
	CharArrayWrapper *new_caw;
	bitlen += this->get_pos_bit();
	if (bitlen % 8 == 0) {
		req_bytelen = (bitlen / 8);
	} else {
		req_bytelen = (bitlen / 8) + 1;
	}
	new_caw = this->caw_from_caw(req_bytelen);
	if (new_caw == NULL) {
		return NULL;
	}
	bitlen -= this->get_pos_bit(); // Remove adjustment bits again
	this->skip(bitlen / 8, bitlen % 8);
	return new_caw;
}

CharArrayWrapper::CharArrayWrapper(uint8_t *mem_ptr, size_t mem_len):
	mem_ptr(mem_ptr),
	mem_len(mem_len),
	bitbuf(0),
	bitbuf_len(0),
	bytepos(0),
	ERRORLEVEL(0),
	free_on_dealloc(1)
	{}

CharArrayWrapper::~CharArrayWrapper() {
	if (this->free_on_dealloc != 0) {
		free(this->mem_ptr);
		this->mem_ptr = NULL;
	}
}

uint8_t CharArrayWrapper::_ver_buf_health(size_t req_bytes, uint8_t req_bits) {
		if (req_bits > 7)
			return 2;
		// Temp var in case bitbuf overflows
		uint8_t tmp;
		if (req_bits > this->bitbuf_len) { tmp = 1; } else { tmp = 0; }
		if ((this->mem_len - this->bytepos) < (req_bytes + tmp))
			return 1;
		return 0;
	}

void CharArrayWrapper::read_raw(void *target_ptr_, size_t req_bytes, uint8_t req_bits) {
	uint8_t carry; // temporary bit level storage
	size_t i; // Loop variable

	uint8_t *target_ptr = (uint8_t *)target_ptr_;

	if (this->_ver_buf_health(req_bytes, req_bits) != 0) {
		this->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}
	if (this->bitbuf_len == 0) { // No bit offset; Should work faster
		memcpy(target_ptr, (void *)(this->mem_ptr + this->bytepos), req_bytes);
		this->bytepos += req_bytes;
		if (req_bits != 0) {
			memset(
				target_ptr + req_bytes,
				this->mem_ptr[this->bytepos] & ((1 << req_bits) - 1),
				1
			);
			this->bitbuf = this->mem_ptr[this->bytepos] >> req_bits;
			this->bitbuf_len = (8 - req_bits);
			this->bytepos += 1;
		}
	} else { // Offset on bitlevel, shifting required
		carry = this->bitbuf;
		for (i = 0; i < req_bytes; i++) {
			memset(
				target_ptr + i,
				carry | (this->mem_ptr[this->bytepos + i] << this->bitbuf_len),
				1
			);
			carry = this->mem_ptr[this->bytepos + i] >> (8 - this->bitbuf_len);
		}
		this->bytepos += req_bytes;
		this->bitbuf = carry;
		if (req_bits != 0) { // Otherwise done
			// Throw extra char at target_ptr, change bitbuf_len, update bitbuf
			if (req_bits <= this->bitbuf_len) { // Drain bitbuf of required bits
				memset(target_ptr + req_bytes, this->bitbuf & ((1 << req_bits) - 1), 1);
				this->bitbuf >>= req_bits;
				this->bitbuf_len -= req_bits;
			} else { // "Read" another byte from chararray
				// i. e. [0b[00000]100, 3], but i want 4 bits
				// so read the next byte into carry: i.e. 10010001
				// using fancy ops 2 lines below: [0000]1100
				// then save [0b[0]1001000, 7] as new bitbuf
				carry = this->mem_ptr[this->bytepos];
				memset(
					target_ptr + req_bytes,
					this->bitbuf | ((carry << this->bitbuf_len) & ((1 << req_bits) - 1)),
					1
				);
				// Ok because req_bits > bitbuf_len in this else block
				this->bitbuf = carry >> (req_bits - this->bitbuf_len);
				this->bitbuf_len = (8 - (req_bits - this->bitbuf_len));
				this->bytepos += 1;
			}
		}
	}
}

void CharArrayWrapper::skip(size_t bytes, uint8_t bits) {
	if (this->_ver_buf_health(bytes, bits) != 0) {
		this->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}

	this->bytepos += bytes; // Skip bytes

	if (this->bitbuf_len != 0) { // Adjust bitbuffer (length remains)
		this->bitbuf = (this->mem_ptr[this->bytepos - 1]) >> (8 - this->bitbuf_len);
	}

	if (bits != 0) {
		if (bits <= this->bitbuf_len) { // Shorten bitbuf
			this->bitbuf >>= bits;
			this->bitbuf_len -= bits;
		} else { // "Read" next byte from chararray
			this->bitbuf = (this->mem_ptr[this->bytepos] >> (bits - this->bitbuf_len));
			this->bitbuf_len = (8 - (bits - this->bitbuf_len));
			this->bytepos += 1;
		}
	}
}

size_t CharArrayWrapper::get_pos_byte() {
	if (this->bitbuf_len == 0) { // Next unread byte is basically position
		return this->bytepos;
	} else { // Untouched data in bitbuffer -> pos is still in current byte
		return this->bytepos - 1;
	}
}

uint8_t CharArrayWrapper::get_pos_bit() {
	if (this->bitbuf_len == 0) {
		return 0;
	} else {
		return (8 - this->bitbuf_len);
	}
	// 0 0 1 0 1 1|0 1
	// -----6-----|-2-
	// -> pos = 8 - 6 = 2
}

void CharArrayWrapper::set_pos(size_t byte, uint8_t bit) {
	if (bit > 7) { return; }
	if (byte > this->mem_len) {
		this->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
		return;
	}
	this->bytepos = byte;
	this->bitbuf = 0;
	this->bitbuf_len = 0;

	this->skip(0, bit);
}

size_t CharArrayWrapper::dist_until_null() {
	size_t c_ln = 0;
	uint8_t cur_byte;
	size_t maxdist = this->mem_len - this->bytepos;
	size_t i = 0;
	uint8_t carry = this->bitbuf;
	if (this->bitbuf_len == 0) {
		for (i = 0; i < maxdist; i++) {
			cur_byte = this->mem_ptr[this->bytepos + i];
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
		}
	} else {
		for (i = 0; i < maxdist; i++) {
			// Current byte is bitbuffer ORed with the next unread byte shifted to fill rest of bitbuffer
			cur_byte = carry | ((this->mem_ptr[this->bytepos + i]) << (this->bitbuf_len));
			c_ln += 1;
			if (cur_byte == 0x00) {
				goto dist_until_null_break;
			}
			carry = ((this->mem_ptr[this->bytepos + i]) >> (8 - this->bitbuf_len));
		}
	}
	this->ERRORLEVEL |= CAW_ERR_BUFFER_TOO_SHORT;
dist_until_null_break:
	return c_ln;
}

uint8_t CharArrayWrapper::remaining_bits() {
	return this->bitbuf_len;
}

size_t CharArrayWrapper::remaining_bytes() {
	return (this->mem_len - this->bytepos);
}

char *CharArrayWrapper::get_chars(size_t req_len) {
	char *res_ptr = (char *)malloc(req_len + 1);
	if (res_ptr == NULL) {
		this->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	this->read_raw(res_ptr, req_len, 0);
	res_ptr[req_len] = '\00';
	return res_ptr;
}

char *CharArrayWrapper::get_nulltrm_str() {
	char *res_ptr;
	size_t ntstr_ln = this->dist_until_null();
	if (this->ERRORLEVEL & CAW_ERR_BUFFER_TOO_SHORT) { // set by method above
		return NULL;
	}
	res_ptr = (char *)malloc(ntstr_ln);
	if (res_ptr == NULL) {
		this->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
		return NULL;
	}
	this->read_raw(res_ptr, ntstr_ln, 0);
	return res_ptr;
}

char *CharArrayWrapper::get_chars_up_to_null(size_t len) {
	char *res_ptr;
	size_t ntstr_ln = this->dist_until_null();
	if (this->ERRORLEVEL & CAW_ERR_BUFFER_TOO_SHORT) { // set by method above
		return NULL;
	}
	if (ntstr_ln < len) { // Pick the shorter one
		res_ptr = (char *)malloc(ntstr_ln);
		if (res_ptr == NULL) {
			this->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
			return NULL;
		}
		this->read_raw(res_ptr, ntstr_ln, 0);
		this->skip(len - ntstr_ln, 0);
	} else {
		res_ptr = (char *)malloc(len + 1);
		if (res_ptr == NULL) {
			this->ERRORLEVEL |= CAW_ERR_MEMORY_ALLOCATION;
			return NULL;
		}
		this->read_raw(res_ptr, len, 0);
		res_ptr[len] = '\00';
	}
	return res_ptr;
}

uint32_t CharArrayWrapper::get_var_int() {
	uint32_t res = 0;
	uint8_t i = 0;
	uint8_t read;
	for (i = 0; i < 35; i += 7) {
		read = this->get_uint8();
		res |= ((read & 0x7F) << i);
		if ((read >> 7) == 0) {
			break;
		}
	}
	return res;
}

float CharArrayWrapper::get_bit_coord() {
	uint8_t has_dec, has_frac;
	uint8_t frac_val = 0;
	int8_t sign;
	uint16_t decimal_val = 0;
	has_dec = this->get_bit();
	has_frac = this->get_bit();
	if (!(has_dec | has_frac)) {
		return 0.0f;
	}
	if (this->get_bit() == 1) {
		sign = 1;
	} else {
		sign = -1;
	}
	if (has_dec == 1) {
		this->read_raw(&decimal_val, 1, 6);
	}
	if (has_frac == 1) {
		this->read_raw(&frac_val, 0, 5);
	}
	return sign * (decimal_val + (frac_val * 0.03125f));
}

float CharArrayWrapper::get_flt() {
	float f;
	this->read_raw(&f, 4, 0);
	return f;
}

uint8_t CharArrayWrapper::get_bit() {
	uint8_t i;
	this->read_raw(&i, 0, 1);
	return i;
}

uint8_t CharArrayWrapper::get_uint8() {
	uint8_t i;
	this->read_raw(&i, 1, 0);
	return i;
}

uint16_t CharArrayWrapper::get_uint16() {
	uint16_t i;
	this->read_raw(&i, 2, 0);
	return i;
}

uint16_t CharArrayWrapper::get_int16() {
	int16_t i;
	this->read_raw(&i, 2, 0);
	return i;
}

uint32_t CharArrayWrapper::get_uint32() {
	uint32_t i;
	this->read_raw(&i, 4, 0);
	return i;
}

uint32_t CharArrayWrapper::get_int32() {
	int32_t i;
	this->read_raw(&i, 4, 0);
	return i;
}

uint8_t CharArrayWrapper::get_errorlevel() {
	return this->ERRORLEVEL;
}
