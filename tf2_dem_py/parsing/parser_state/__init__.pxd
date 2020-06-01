from libc.stdint cimport uint8_t, uint16_t, uint32_t

from tf2_dem_py.parsing.game_events cimport GameEventDefinitionArray

"""
finished: 0 if parser is not done with parsing the demo, 1 else.
FAILURE:
	0b 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
	   |               | | | | | | | \\ CharArrayWrapper failed. See RELAYED_CAW_ERR for details.
	   |               | | | | | | \\ Unknown packet id encountered.
	   |               | | | | | \\ File access error.
	   |               | | | | \\ Unexpected EOF.
	   |               | | | \\ Unknown message id encountered.
       |               | | \\ Memory allocation error.
	   |               | \\ Python dictionary failure. Exact cause probably memory allocation.
	   |               \\ Python list failure. Exact cause probably memory allocation.
	   \\ Unknown error.

RELAYED_CAW_ERR: See CharArrayWrapper error
"""

cdef extern from "tf2_dem_py/parsing/parser_state/parser_state.h":
	cdef struct ParserState:
		uint16_t flags
		uint8_t finished
		uint8_t FAILURE
		uint8_t RELAYED_CAW_ERR
		uint32_t tick
		GameEventDefinitionArray *game_event_defs
		uint8_t current_message_contains_senderless_chat

	cdef struct ERR_s: # Only required for constant decl. below
		uint8_t CAW
		uint8_t UNKNOWN_PACKET_ID
		uint8_t IO
		uint8_t UNEXPECTED_EOF
		uint8_t CJSON
		uint8_t UNKNOWN_MESSAGE_ID
		uint8_t MEMORY_ALLOCATION
		uint8_t UNKNOWN_GAME_EVENT

	const ERR_s ERR # export constant error struct
