cmake_minimum_required(VERSION 3.9)
project(bench)

set(CMAKE_CXX_STANDARD 14)

add_executable(bench main.cpp abstract.h bitset.h data.h definition.h hash.h fr.h fr_cf.h)
