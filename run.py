import os
from pprint import pprint

from tf2_dem_py.demo_parser import DemoParser

p = os.path.join(os.environ["USERPROFILE"], "Desktop", "demotesting", "lamo.dem")

dp = DemoParser(p)
pprint(dp.parse())
