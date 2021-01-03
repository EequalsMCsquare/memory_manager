cmake_minimum_required(VERSION 3.14)
project(memory_manager)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(fmt REQUIRED)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/bins SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_library(memory_manager STATIC ${SRC})
target_link_libraries(memory_manager PUBLIC fmt::fmt)
target_include_directories(memory_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BUILD_MEM_TEST "build with testcases" ON)

if (BUILD_MEM_TEST)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()