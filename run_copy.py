
# Shoddy directory-wise run script

import os
import json
import sys
from pprint import pprint

from bytes_encoder import BytesEncoder

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser

p = sys.argv[1]

for i, file in enumerate([os.path.join(p, i) for i in os.listdir(p) if i.endswith(".dem")]):
	print(file)
	dp = DemoParser(FLAGS.CHAT | FLAGS.COMPACT_CHAT)
	res = dp.parse(file)
	# pprint(res, sort_dicts = False, compact = True)

	with open(f"lastdemo{i}.json", "w") as h:
		json.dump(res, h, cls = BytesEncoder)

	# print(res)
