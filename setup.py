import glob
import os
import re
from pathlib import Path
from setuptools import setup, Extension

# Thanks: https://stackoverflow.com/questions/2058802/
# 	how-can-i-get-the-version-defined-in-setup-py-setuptools-in-my-package
__version__ = None
with open("tf2_dem_py/demo_parser.cpp") as h:
	for line in h.readlines():
		if line.startswith("#define _tf2_dem_py__version__"):
			__version__ = re.search('"(.*)"', line) # very cheap but why would the version have multiple quotes in it?
			break

if __version__ == None:
	raise SyntaxError("Version not found.")
__version__ = __version__[1]

SRC_CAW = "tf2_dem_py/char_array_wrapper/char_array_wrapper.cpp"
SRC_FLAGS = "tf2_dem_py/flags/flags.c"
SRC_HEADER = "tf2_dem_py/parsing/demo_header.cpp"
SRC_PARSER_STATE = "tf2_dem_py/parsing/parser_state/parser_state.cpp"
SRCS_MSG = glob.glob("tf2_dem_py/parsing/message/*.cpp")
SRCS_PACKETS = glob.glob("tf2_dem_py/parsing/packet/*.cpp")
SRCS_USERMSG = glob.glob("tf2_dem_py/parsing/usermessage/*.cpp")

def deliver_sources(strpath):
	srcs = [strpath]
	path = Path(strpath)
	if path.match("tf2_dem_py/demo_parser.cpp"):
		srcs.append(SRC_CAW)
		srcs.append(SRC_FLAGS)
		srcs.append(SRC_HEADER)
		srcs.append(SRC_PARSER_STATE)
		srcs.extend(SRCS_MSG)
		srcs.extend(SRCS_PACKETS) ##
	return srcs

extensions = [
	Extension(
		"tf2_dem_py.demo_parser",
		sources = deliver_sources("tf2_dem_py/demo_parser.cpp"),
		extra_compile_args = ["-DMS_WIN64"],
		extra_link_args = ["-static", "-static-libgcc", "-static-libstdc++"],
	)
]

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	packages = ["tf2_dem_py"],
	include_dirs = [ # This is so incredibly hardcoded lmao
            "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0" \
            	"/mingw64/x86_64-w64-mingw32/include",
			"C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0" \
            	"/mingw64/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++",
			"C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0/" \
				"mingw64/lib/gcc/x86_64-w64-mingw32/8.1.0/include/c++/x86_64-w64-mingw32",
			".",
		],
	ext_modules = extensions,
)
