import os

from tf2_dem_py.demo_parser import DemoParser

p = os.path.join(os.environ["USERPROFILE"], "Desktop", "demotesting", "testdemo.dem")

dp = DemoParser(p)
print(dp.parse())
