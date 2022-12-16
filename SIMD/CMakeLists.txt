cmake_minimum_required(VERSION 3.9)
project(Bench)

set(CMAKE_CXX_STANDARD 17)

add_compile_options(-march=native -O2)
add_executable(bench main.cpp abstract.h hash.h SS.h USS.h StreamSummary.h Heap.h Count_Heap.h benchmark.h WavingSketch.h)
