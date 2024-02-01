cmake_minimum_required(VERSION 3.9)
project(bench)

set(CMAKE_CXX_STANDARD 14)

add_executable(bench main.cpp abstract.h benchmark.h bitset.h data.h definition.h hash.h fr.h fr_cf.h ld.h LD-Sketch/hash.c LD-Sketch/hash.h LD-Sketch/dyn_tbl.hpp LD-Sketch/dyn_tbl.cpp LD-Sketch/util.h LD-Sketch/LDSketch.hpp LD-Sketch/LDSketch.cpp)
