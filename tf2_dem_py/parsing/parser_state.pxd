from libc.stdint cimport uint8_t, uint16_t, uint32_t

"""
finished: 0 if parser is not done with parsing the demo, 1 else.
FAILURE:
	0b 0 0 0 0 0 0 0 0
	     | | | | | | \\ CharArrayWrapper failed. See RELAYED_CAW_ERR for details.
	     | | | | | \\ Unknown packet id encountered.
	     | | | | \\ File access error.
	     | | | \\ Unexpected EOF.
		 | | \\ cJSON error.
		 | \\ Unknown message id encountered.
         \\ Memory allocation error.

RELAYED_CAW_ERR: See CharArrayWrapper error
"""

cdef extern from "tf2_dem_py/parsing/parser_state.h":
	cdef struct ParserState:
		uint16_t flags
		uint8_t finished
		uint8_t FAILURE
		uint8_t RELAYED_CAW_ERR
		uint32_t tick
