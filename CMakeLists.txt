cmake_minimum_required(VERSION 3.16)
project(MayhemEngine CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(MDBG_ENABLED "Enable MayhemDebugger instrumentation (OFF compiles all DEBUG_* macros down to the bare condition)" ON)

add_subdirectory(modules/MayhemDebugger)
