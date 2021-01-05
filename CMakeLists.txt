cmake_minimum_required(VERSION 3.14)
project(memory_manager)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(shared_memory REQUIRED)

if(shared_memory_FOUND)
  message(STATUS "libshared_memory found! ${shared_memory_VERSION}")
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/bins SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_library(memory_manager STATIC ${SRC})
target_link_libraries(memory_manager PUBLIC spdlog::spdlog shared_memory::shared_memory)
target_include_directories(memory_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BUILD_MEM_TEST "build with testcases" ON)

if (BUILD_MEM_TEST)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()