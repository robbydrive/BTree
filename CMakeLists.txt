cmake_minimum_required(VERSION 3.8)
project(B_Tree)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES btree.h example.c)
add_executable(B_Tree ${SOURCE_FILES})