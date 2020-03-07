@echo off
rmdir build /S /Q 
del tf2_dem_py\parsing\*.pyd
del tf2_dem_py\parsing\*.c
del tf2_dem_py\parsing\packet\*.pyd
del tf2_dem_py\parsing\packet\*.c
del tf2_dem_py\parsing\message\*.pyd
del tf2_dem_py\parsing\message\*.c
