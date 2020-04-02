
from libc.stdint cimport uint16_t

cdef extern from "tf2_dem_py/flags/flags.h":
	cdef struct FLAGS_s: # Needed for const struct below
		uint16_t CHAT
		uint16_t GAME_EVENTS

	const FLAGS_s FLAGS
