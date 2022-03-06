@ECHO OFF
if not exist "build" mkdir "build"
clang hmap_test.c hmap/hmap.c --output build/hmap_test.exe
start /B build/hmap_test.exe
PAUSE