import ast
import glob
import os
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

extensions = []

for i in glob.glob("tf2_dem_py/**/*.pyx", recursive = True):
	if os.path.split(i)[1] == "__init__.pyx":
		continue
	extensions.append(Extension(
		os.path.splitext(i)[0].replace(os.path.sep, "."),
		sources = [
			i,
			"tf2_dem_py/cJSON/cJSON.c", # Apparently required, idk in what way
		],
		extra_compile_args = ["-DMS_WIN64"],
	))

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	packages = ["tf2_dem_py"],
	include_dirs = [
            "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0"
            "/mingw64/x86_64-w64-mingw32/include",
			"tf2_dem_py/cJSON", # Required for "extern from" stmt in cJSON_wrapper.pxd
		],
	ext_modules = cythonize(
		extensions,
		language_level = 3,
		include_path = ["."], # Not .h files, but pxd
		verbose = 1,
	)
)
