import os

from tf2_dem_py.demo_parser import PyDemoParser

p = os.path.join(os.environ["USERPROFILE"], "Desktop", "demotesting", "small.dem")

dp = PyDemoParser(p)
print(dp.parse())
