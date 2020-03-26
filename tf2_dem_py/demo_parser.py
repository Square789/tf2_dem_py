"""
Main parser class
"""
from tf2_dem_py.parsing.cy_demo_parser import CyDemoParser

__version__ = "0.0.1-dev-19"

class DemoParser():
	def __init__(self, target, flags):
		"""
		target: Path to target demo
		flags: Flag number constructed from bitflags
		"""
		flags &= 0xFFFF
		self.parser = CyDemoParser(
			bytes(target, encoding = "utf-8"),
			flags,
		)

	def parse(self):
		"""
		Parse the demo, returning some dict. i have no idea what is gonna be in there lol
		"""
		return self.parser.parse()
