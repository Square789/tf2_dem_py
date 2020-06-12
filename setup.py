import ast
import glob
import os
from pathlib import Path
from setuptools import setup, Extension
from Cython.Build.Cythonize import cythonize

# Thanks: https://stackoverflow.com/questions/2058802/
# 	how-can-i-get-the-version-defined-in-setup-py-setuptools-in-my-package
__version__ = None
with open("tf2_dem_py/demo_parser.pyx") as h:
	for line in h.readlines():
		if line.startswith("__version__"):
			__version__ = ast.parse(line).body[0].value.s
			break

if __version__ == None:
	raise SyntaxError("Version not found.")

SRC_CAW = "tf2_dem_py/char_array_wrapper/char_array_wrapper.cpp"
SRC_CJSON = "tf2_dem_py/cJSON/cJSON.c"
SRC_CONSTAMTS = "tf2_dem_py/constants.cpp"
SRC_FLAGS = "tf2_dem_py/flags/flags.c"
SRC_GAME_EVENTS = "tf2_dem_py/parsing/game_events/game_events.cpp"
SRC_HEADER = "tf2_dem_py/parsing/demo_header.cpp"
SRC_PARSER_STATE = "tf2_dem_py/parsing/parser_state/parser_state.c"
SRCS_MSG = glob.glob("tf2_dem_py/parsing/message/*.cpp")
SRCS_PACKETS = glob.glob("tf2_dem_py/parsing/packet/*.cpp")
SRCS_USERMSG = glob.glob("tf2_dem_py/parsing/usermessage/*.cpp")

def deliver_sources(strpath):
	srcs = [strpath]
	path = Path(strpath)
	if path.match("tf2_dem_py/demo_parser.cpp"):
		srcs.append(SRC_CAW)
		#srcs.append(SRC_CJSON)
		srcs.append(SRC_CONSTAMTS)
		srcs.append(SRC_FLAGS)
		srcs.append(SRC_GAME_EVENTS)
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

# for i in glob.glob("tf2_dem_py/**/*.pyx", recursive = True):
# 	if os.path.split(i)[1] == "__init__.pyx":
# 		continue
# 	if "packet__" in i:
# 		continue
# 	extensions.append(Extension(
# 		os.path.splitext(i)[0].replace(os.path.sep, "."),
# 		sources = deliver_sources(i),
# 		extra_compile_args = ["-DMS_WIN64"],
# 	))

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	packages = ["tf2_dem_py"],
	include_dirs = [
            "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0"
            "/mingw64/x86_64-w64-mingw32/include",
			".",
		],
	ext_modules = extensions,
)
