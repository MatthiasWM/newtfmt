
cmake_minimum_required(VERSION 3.15.0 FATAL_ERROR)

project( newtfmt )

# Fix behavior of CMAKE_CXX_STANDARD when targeting macOS.
if (POLICY CMP0025)
  cmake_policy(SET CMP0025 NEW)
endif ()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SRCS
  src/main.cpp
)

set(PACKAGE_SRCS
  src/package/package_bytes.h
  src/package/package_bytes.cpp
  src/package/package.h
  src/package/package.cpp
  src/package/relocation_data.h
  src/package/relocation_data.cpp
  src/package/part_entry.h
  src/package/part_entry.cpp
  src/package/part_data.h
  src/package/part_data.cpp
)

set(NOS_SRCS
  src/nos/types.h
  src/nos/types.cpp  
  src/nos/objects.h
  src/nos/objects.cpp
  src/nos/ref.h
  src/nos/ref.cpp
  src/nos/print.h
  src/nos/print.cpp
)

set(TOOLS_SRCS
  src/tools/tools.h
  src/tools/tools.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
    ${SRCS}
    ${PACKAGE_SRCS}
    ${NOS_SRCS}
    ${TOOLS_SRCS}
)

add_executable(
  # executable name
  newtfmt
  # source files
  ${SRCS}
  ${PACKAGE_SRCS}
  ${NOS_SRCS}
  ${TOOLS_SRCS}
  # other files for quick access
  README.md
  Package.md
)

set_source_files_properties(README.md PROPERTIES HEADER_FILE_ONLY TRUE)
set_source_files_properties(Package.md PROPERTIES HEADER_FILE_ONLY TRUE)

target_include_directories(
  newtfmt
  PUBLIC
    src
)

if(MSVC)
  target_compile_options(newtfmt PRIVATE /W4 /WX)
else()
  target_compile_options(newtfmt PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif()
