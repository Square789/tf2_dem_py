from libc.stdio cimport FILE, fread
from libc.stdint cimport uint32_t

cdef struct s_header:
	char ident[8]
	uint32_t dem_prot
	uint32_t net_prot
	char host_name[260]
	char client_id[260]
	char map_name[260]
	char game_dir[260]

ctypedef s_header Header

cdef inline Header parse(FILE *stream):
	cdef Header h
	fread((&h.ident), 8, 1, stream)
	fread((&h.dem_prot), 4, 1, stream)
	fread((&h.net_prot), 4, 1, stream)
	fread((&h.host_name), 260, 1, stream)
	fread((&h.client_id), 260, 1, stream)
	fread((&h.map_name), 260, 1, stream)
	fread((&h.game_dir), 260, 1, stream)
	cdef dict hd = {}
	hd["ident"] = (h.ident).decode("utf-8")
	hd["dem_prot"] = int(<unsigned int> h.dem_prot)
	hd["net_prot"] = int(<unsigned int> h.net_prot)
	hd["host_name"] = (h.host_name).decode("utf-8")
	hd["client_id"] = (h.client_id).decode("utf-8")
	hd["map_name"] = (h.map_name).decode("utf-8")
	hd["game_dir"] = (h.game_dir).decode("utf-8")
	# hd["ident"] = 
	# hd["ident"] = 
	# hd["ident"] = 
	return h
