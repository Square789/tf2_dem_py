
# Shoddy run script

import os
import json
import sys
from pprint import pprint

from bytes_encoder import BytesEncoder

import tf2_dem_py.demo_parser
from tf2_dem_py.flags import FLAGS

if __name__ == "__main__":
	if len(sys.argv) > 1:
		p = sys.argv[1]
	else:
		p = os.path.join(
			os.environ["USERPROFILE"], "Documents", "demos",
			"autodemo2019-08-18_14-36-22.dem"
		)

	dp = tf2_dem_py.demo_parser.DemoParser(
		FLAGS.CHAT | FLAGS.COMPACT_CHAT | FLAGS.GAME_EVENTS | FLAGS.COMPACT_GAME_EVENTS
	)
	print("Demo parser constructed and it didn't segfault!")
	res = dp.parse(p)
	print("Returned to python")

	# pprint(res, sort_dicts = False, compact = True)

	with open("lastdemo.json", "w") as h:
		json.dump(res, h, indent = "\t", cls = BytesEncoder)
