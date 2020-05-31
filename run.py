
# Shoddy run script

import os
import json
import sys
from pprint import pprint

from tf2_dem_py.flags import FLAGS
import tf2_dem_py.demo_parser

if len(sys.argv) > 1:
	p = sys.argv[1]
else:
	p = os.path.join(os.environ["USERPROFILE"], "Documents", "demos", "autodemo2019-08-17_21-35-44.dem")

dp = tf2_dem_py.demo_parser.DemoParser(
	(
		FLAGS.CHAT |
		FLAGS.GAME_EVENTS
	)
)
res = dp.parse(p)
# pprint(res, sort_dicts = False, compact = True)

with open("lastdemo.json", "w") as h:
	json.dump(res, h)

# print(res)
