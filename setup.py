from setuptools import setup, Extension
from Cython.Build.Cythonize import cythonize


__version__ = "0.0.1-dev"

extensions = (
	Extension(
        "tf2_dem_py.parsing._parser",
        sources = [
		    "tf2_dem_py/parsing/_parser.pyx",
        ],
		include_dirs = [
            "C:/Program Files/mingw-w64/x86_64-8.1.0-posix-seh-rt_v6-rev0"
            "/mingw64/x86_64-w64-mingw32/include",
		]
	),
)

for i in extensions:
	i.extra_compile_args = ["-DMS_WIN64"]

setup(
	name = "tf2_dem_py",
	version = __version__,
	author = "Square789",
	ext_modules = cythonize(extensions,
		language_level = 3,
	)
)