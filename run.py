
# Shoddy run script, pay no attention

import os
import json
import sys
from pprint import pprint

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser

if len(sys.argv) > 1:
	p = sys.argv[1]
else:
	p = os.path.join(os.environ["USERPROFILE"], "Desktop", "demotesting", "gully.dem")

dp = DemoParser(
	bytes(p, encoding = "utf-8"),
	(
		#FLAGS.CHAT |
		FLAGS.GAME_EVENTS
	)
)
res = dp.parse()
#pprint(res, sort_dicts = False, compact = True)

with open("lastdemo.json", "w") as h:
	json.dump(res, h)

#print(res)
