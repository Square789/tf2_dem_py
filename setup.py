#!/usr/bin/env python3

import glob
import re
from pathlib import Path
from setuptools import setup, Extension

# Thanks: https://stackoverflow.com/questions/2058802/
# 	how-can-i-get-the-version-defined-in-setup-py-setuptools-in-my-package
__version__ = None
with open("tf2_dem_py/demo_parser.c") as h:
	for line in h.readlines():
		if line.startswith("#define _tf2_dem_py__version__"):
			__version__ = re.search('"(.*)"', line) # very cheap but why would the version have multiple quotes in it anyways?
			break

if __version__ is None:
	raise SyntaxError("Version not found.")
__version__ = __version__[1]

SRC_CAW = "tf2_dem_py/char_array_wrapper/char_array_wrapper.c"
SRC_CONSTANTS = "tf2_dem_py/constants.c"
SRC_FLAGS = "tf2_dem_py/flags/flags.c"
SRC_HELPERS = "tf2_dem_py/demo_parser/helpers.c"
SRC_PACKET = "tf2_dem_py/demo_parser/packet.c"
SRC_PARSER_STATE = "tf2_dem_py/demo_parser/parser_state.c"
SRCS_DATA_STRUCTS = glob.glob("tf2_dem_py/demo_parser/data_structs/*.c")
SRCS_MSG = glob.glob("tf2_dem_py/demo_parser/message/*.c")
SRCS_USERMSG = glob.glob("tf2_dem_py/demo_parser/usermessage/*.c")

def deliver_sources(strpath):
	srcs = [strpath]
	path = Path(strpath)
	if path.match("tf2_dem_py/demo_parser.c"):
		srcs.append(SRC_CAW)
		srcs.append(SRC_CONSTANTS)
		srcs.append(SRC_FLAGS)
		srcs.append(SRC_HELPERS)
		srcs.append(SRC_PACKET)
		srcs.append(SRC_PARSER_STATE)
		srcs.extend(SRCS_DATA_STRUCTS)
		srcs.extend(SRCS_MSG)
		srcs.extend(SRCS_USERMSG)
	return srcs

extensions = [
	Extension(
		"tf2_dem_py.demo_parser",
		sources = deliver_sources("tf2_dem_py/demo_parser.c"),
		# Yeah yeah, it seems to work okay
		extra_compile_args = ["-Wall", "-Wextra", "-Wno-pointer-sign", "-Wno-unused-parameter"],
		#extra_link_args = ["-static"],
	)
]

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	packages = ["tf2_dem_py"],
	include_dirs = [ # This is so incredibly hardcoded lmao
		"C:/Program_Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0" \
			"/mingw64/x86_64-w64-mingw32/include",
		"C:/Program_Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0" \
			"/mingw64/lib/gcc/x86_64-w64-mingw32/8.1.0/include",
		".",
	],
	ext_modules = extensions,
)
