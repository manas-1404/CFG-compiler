cmake_minimum_required(VERSION 3.30)
project(project2)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_BUILD_TYPE Debug)

add_executable(project2 project2.cc lexer.cc inputbuf.cc)
