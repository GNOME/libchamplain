# Fake CMake file to skip the boring and slow stuff
set(CMAKE_C_COMPILER "/usr/lib/python3.7/site-packages/mesonbuild/dependencies/base.py") # Just give CMake a valid full path to any file
set(CMAKE_C_COMPILER_ID "GNU") # Pretend we have found GCC
set(CMAKE_COMPILER_IS_GNUCC 1)
set(CMAKE_C_COMPILER_LOADED 1)
set(CMAKE_C_COMPILER_WORKS TRUE)
set(CMAKE_C_ABI_COMPILED TRUE)
set(CMAKE_SIZEOF_VOID_P "8")
