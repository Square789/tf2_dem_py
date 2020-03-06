"""
Main parser class
"""
from tf2_dem_py.parsing.cy_demo_parser import CyDemoParser

class PyDemoParser():
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
		print("ayy")
		return self.parser.parse()