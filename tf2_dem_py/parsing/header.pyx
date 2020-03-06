from libc.stdio cimport FILE

from tf2_dem_py.parsing.chararray_wrapper cimport CharArrayWrapper

cdef dict parse(FILE *stream):
	cdef dict hdict = {}
	cdef CharArrayWrapper header_stream = \
		CharArrayWrapper.create_new(stream, 1260)

	hdict["ident"] = header_stream.get_utf8_str(8)
	hdict["net_prot"] = header_stream.get_uint32()

	return hdict

# cdef dict parse(FILE* stream):
# 	# print("header parser meth called")
# 	# cdef CyCAW header_stream
# 	# header_stream = CyCAW.create_new(stream, 1260)

# 	# fread((&h.ident), 8, 1, stream)
# 	# fread((&h.dem_prot), 4, 1, stream)
# 	# fread((&h.net_prot), 4, 1, stream)
# 	# fread((&h.host_name), 260, 1, stream)
# 	# fread((&h.client_id), 260, 1, stream)
# 	# fread((&h.map_name), 260, 1, stream)
# 	# fread((&h.game_dir), 260, 1, stream)

# 	# hd["dem_prot"] = int(<unsigned int> h.dem_prot)
# 	# hd["net_prot"] = int(<unsigned int> h.net_prot)
# 	# hd["host_name"] = (h.host_name).decode("utf-8")
# 	# hd["client_id"] = (h.client_id).decode("utf-8")
# 	# hd["map_name"] = (h.map_name).decode("utf-8")
# 	# hd["game_dir"] = (h.game_dir).decode("utf-8")

# 	return hd
