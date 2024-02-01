cmake_minimum_required(VERSION 3.9)
project(bench)

set(CMAKE_CXX_STANDARD 14)
set(CXX_FLAGS -O2)

add_executable(bench main.cpp abstract.h hash.h SS.h USS.h StreamSummary.h Heap.h Count_Heap.h benchmark.h WavingSketch.h ld.h LD-Sketch/hash.c LD-Sketch/hash.h LD-Sketch/dyn_tbl.hpp LD-Sketch/dyn_tbl.cpp LD-Sketch/util.h LD-Sketch/LDSketch.hpp LD-Sketch/LDSketch.cpp)
