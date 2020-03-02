"""
Main parser class
"""
from tf2_dem_py.parsing import _parser

class DemoParser():
	def __init__(self, target):
		"""
		target: Path to target demo
		"""
		self.parser = _parser.DemoParser(
			bytes(target, encoding = "utf-8"),
		)

	def parse(self):
		"""
		Parse the demo, returning some dict that i have no idea what is gonna be in there lol
		"""
		return self.parser.parse()