
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project( newtfmt )

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
  newtfmt
  newtfmt/main.cpp
)
