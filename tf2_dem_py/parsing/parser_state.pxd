from libc.stdint cimport uint8_t

cdef struct ParserState:
	uint8_t finished
	uint8_t FAILURE
	uint8_t RELAYED_CAW_ERR
