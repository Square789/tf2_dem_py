"""
Contains the flag class, complete with flags to be passed to a demo
parser upon instantiation.
"""

class FLAGS:
	CHAT = (1 << 0)
	GAME_EVENTS = (1 << 1)
	COMPACT_CHAT = (1 << 2)
	COMPACT_GAME_EVENTS = (1 << 3)
