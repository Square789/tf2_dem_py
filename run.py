
# Shoddy run script, pay no attention

import os
import sys
from pprint import pprint

from tf2_dem_py.demo_parser import DemoParser

if len(sys.argv) > 1:
	p = sys.argv[1]
else:
	p = os.path.join(os.environ["USERPROFILE"], "Documents", "demotesting", "small.dem")

dp = DemoParser(p)
pprint(dp.parse())
