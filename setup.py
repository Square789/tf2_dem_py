from setuptools import setup, Extension
from Cython.Build.Cythonize import cythonize


__version__ = "0.0.1dev-0"

extensions = (
	Extension(
        "tf2_dem_py.parsing.chararray_wrapper",
        sources = ["tf2_dem_py/parsing/chararray_wrapper.pyx",],
	),
	Extension(
        "tf2_dem_py.parsing.header",
        sources = ["tf2_dem_py/parsing/header.pyx",],
	),
	Extension(
        "tf2_dem_py.parsing.cy_demo_parser",
        sources = [
			"tf2_dem_py/parsing/cy_demo_parser.pyx",
		],
	),
)

for i in extensions:
	i.extra_compile_args = ["-DMS_WIN64"]
# 	i.include_dirs = [
#             "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0"
#             "/mingw64/x86_64-w64-mingw32/include",
# 		]

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
	# ext_modules = cythonize(
	# 	Extension(
	# 		"",
	# 		["tf2_dem_py/**/*.pyx"],
	# 		extra_compile_args = ["-DMS_WIN64"]
	# 	),
	# 	language_level = 3,
	# 	include_path = ["."], # Not .h files, but pxd
	# 	verbose = 1,
	# )
	# ext_modules = cythonize(["tf2_dem_py/**/*.pyx"],
	# 	language_level = 3,
	# 	include_path = ["."], # Not .h files, but pxd
	# 	verbose = 1,
	# ),
)