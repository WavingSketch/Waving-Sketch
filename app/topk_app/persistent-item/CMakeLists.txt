cmake_minimum_required(VERSION 3.9)
project(bench)

set(CMAKE_CXX_STANDARD 14)

add_executable(bench main.cpp abstract.h benchmark.h bitset.h data.h definition.h hash.h pie.h small_space.h Waving.h on_off_sketch.h)
