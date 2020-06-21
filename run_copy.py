
<<<<<<< Updated upstream
# Shoddy run script
=======
# Shoddy directory-wise run script
>>>>>>> Stashed changes

import os
import json
import sys
from pprint import pprint

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser

p = sys.argv[1]

print(p)

for i, file in enumerate([os.path.join(p, i) for i in os.listdir(p) if i.endswith(".dem")]):
	print(file)
	dp = DemoParser(
<<<<<<< Updated upstream
		bytes(file, encoding = "utf-8"),
=======
>>>>>>> Stashed changes
		(
			FLAGS.CHAT |
			FLAGS.GAME_EVENTS
		)
	)
<<<<<<< Updated upstream
	res = dp.parse()
=======
	res = dp.parse(file)
>>>>>>> Stashed changes
	# pprint(res, sort_dicts = False, compact = True)

	with open(f"lastdemo{i}.json", "w") as h:
		json.dump(res, h)

	# print(res)
