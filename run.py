
# Shoddy run script, pay no attention

import os
import sys
from pprint import pprint

from tf2_dem_py.flags import FLAGS
from tf2_dem_py.demo_parser import DemoParser

if len(sys.argv) > 1:
	p = sys.argv[1]
else:
	p = os.path.join(os.environ["USERPROFILE"], "Desktop", "demotesting", "gully.dem")

dp = DemoParser(p, (FLAGS.CHAT))
res = dp.parse()
#pprint(res, sort_dicts = False, compact = True)
print(res)
