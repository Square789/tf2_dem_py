
# Shoddy directory-wise run script

import os
import json
import sys
from pprint import pprint

from bytes_encoder import BytesEncoder

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser, ParserError

p = sys.argv[1]

for i, file in enumerate(os.path.join(p, i) for i in os.listdir(p) if i.endswith(".dem")):
	print(file)
	dp = DemoParser(FLAGS.CHAT | FLAGS.COMPACT_CHAT)
	try:
		res = dp.parse(file)
	except ParserError as exc:
		print("!!!")
		print(exc)
		print("!!!\n")
		continue
	#
	#  pprint(res, sort_dicts = False, compact = True)

	with open(f"_res/lastdemo{i}.json", "w") as h:
		json.dump(res, h, cls = BytesEncoder)

	with open(f"_res/lastdemo{i}.txt", "w", encoding="utf-8") as h:
		for m in res["chat"]:
			if not m[0]: # Non-normal message
				continue
			t = [None, None, None]
			for i, x in enumerate(m[3:6]):
				try:
					t[i] = x.decode("utf-8")
				except UnicodeDecodeError:
					t[i] = "<DECODING FAILURE>"
			chat, name, text = t
			h.write(f"*{chat:<20}* {name:<31}: {text}\n")

	# print(res)
