@echo off
REM crappy build deletion script, probably defunct. Named by dragging palm over keyboard.
rmdir build /S /Q 
del tf2_dem_py\parsing\*.pyd
del tf2_dem_py\parsing\*.c
del tf2_dem_py\parsing\packet\*.pyd
del tf2_dem_py\parsing\packet\*.c
