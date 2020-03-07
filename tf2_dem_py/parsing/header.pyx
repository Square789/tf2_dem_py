from libc.stdio cimport FILE

from tf2_dem_py.parsing.chararray_wrapper cimport CharArrayWrapper

cdef dict parse(FILE *stream):
	cdef dict hdict = {}
	cdef CharArrayWrapper header_stream = \
		CharArrayWrapper.create_new(stream, 1260)

	hdict["ident"] = header_stream.get_utf8_str(8)
	hdict["net_prot"] = header_stream.get_uint32()
	hdict["dem_prot"] = header_stream.get_uint32()
	hdict["host_addr"] = header_stream.get_utf8_str(260)
	hdict["client_id"] = header_stream.get_utf8_str(260)
	hdict["map_name"] = header_stream.get_utf8_str(260)
	hdict["game_dir"] = header_stream.get_utf8_str(260)

	return hdict
