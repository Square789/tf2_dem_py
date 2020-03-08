from libc.stdint cimport uint8_t

"""
finished: 0 if parser is not done with parsing the demo, 1 else.
FAILURE:
	0b 0 0 0 0 0 0 0 0
	           | | | \\ CharArrayWrapper failed. See RELAYED_CAW_ERR for details.
	           | | \\ Unknown packet id encountered.
	           | \\ File access error.
	           \\ Unexpected EOF.

RELAYED_CAW_ERR: See CharArrayWrapper error
"""

cdef struct ParserState:
	uint8_t finished
	uint8_t FAILURE
	uint8_t RELAYED_CAW_ERR
