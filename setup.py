import ast
from setuptools import setup, Extension
from Cython.Build.Cythonize import cythonize

# Thanks: https://stackoverflow.com/questions/2058802/
# 	how-can-i-get-the-version-defined-in-setup-py-setuptools-in-my-package
__version__ = None
with open("tf2_dem_py/demo_parser.py") as h:
	for line in h.readlines():
		if line.startswith("__version__"):
			__version__ = ast.parse(line).body[0].value.s
			break

if __version__ == None:
	raise SyntaxError("Version not found.")

extensions = (
	Extension(
        "tf2_dem_py.parsing.chararray_wrapper",
        sources = ["tf2_dem_py/parsing/chararray_wrapper.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.header",
        sources = ["tf2_dem_py/parsing/header.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.cy_demo_parser",
        sources = ["tf2_dem_py/parsing/cy_demo_parser.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.parse_any",
        sources = ["tf2_dem_py/parsing/packet/parse_any.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.message",
        sources = ["tf2_dem_py/parsing/packet/message.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.datatables",
        sources = ["tf2_dem_py/parsing/packet/datatables.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.stringtables",
        sources = ["tf2_dem_py/parsing/packet/stringtables.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.synctick",
        sources = ["tf2_dem_py/parsing/packet/synctick.pyx"],
	),
	Extension(
        "tf2_dem_py.parsing.packet.consolecmd",
        sources = ["tf2_dem_py/parsing/packet/consolecmd.pyx"],
	),
)

for i in extensions:
	i.extra_compile_args = ["-DMS_WIN64"]

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	packages = ["tf2_dem_py"],
	include_dirs = [
            "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0"
            "/mingw64/x86_64-w64-mingw32/include",
		],
	ext_modules = cythonize(
		extensions,
		language_level = 3,
		include_path = ["."], # Not .h files, but pxd
		verbose = 1,
	),
	# ext_modules = cythonize(["tf2_dem_py/**/*.pyx"],
	# 	language_level = 3,
	# 	include_path = ["."], # Not .h files, but pxd
	# 	verbose = 1,
	# ),
)
