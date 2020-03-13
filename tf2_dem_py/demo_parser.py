"""
Main parser class
"""
from tf2_dem_py.parsing.cy_demo_parser import CyDemoParser

__version__ = "0.0.1-dev-10"

class DemoParser():
	def __init__(self, target):
		"""
		target: Path to target demo
		"""
		self.parser = CyDemoParser(
			bytes(target, encoding = "utf-8"),
		)

	def parse(self):
		"""
		Parse the demo, returning some dict. i have no idea what is gonna be in there lol
		"""
		return self.parser.parse()
