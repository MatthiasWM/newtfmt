
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project( newtfmt )

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
  # executable name
  newtfmt
  # sourec files
  newtfmt/main.cpp
  newtfmt/package_bytes.h
  newtfmt/package_bytes.cpp
  newtfmt/package.h
  newtfmt/package.cpp
  newtfmt/relocation_data.h
  newtfmt/relocation_data.cpp
  newtfmt/part_entry.h
  newtfmt/part_entry.cpp
  newtfmt/part_data.h
  newtfmt/part_data.cpp
  newtfmt/tools.h
  newtfmt/tools.cpp
  # other files for quick acces
  README.md
)

set_source_files_properties(README.md PROPERTIES HEADER_FILE_ONLY TRUE)
