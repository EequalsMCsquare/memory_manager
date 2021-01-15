cmake_minimum_required(VERSION 3.14)
project(memory_manager
  VERSION 0.2.0
  DESCRIPTION "shm_kernel's memory handler component")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)

if(NOT shared_memory_FOUND)
  find_package(shared_memory REQUIRED)
endif()


if(shared_memory_FOUND)
  message(STATUS "libshared_memory found! ${shared_memory_VERSION}")
else()
  message(ERROR "unalble to find libshared_memory");
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/bins SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_library(memory_manager STATIC ${SRC})
target_link_libraries(memory_manager PUBLIC fmt::fmt spdlog::spdlog shm_kernel::shared_memory)
target_include_directories(memory_manager PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

option(BUILD_MEM_TEST "build with testcases" ON)

if (BUILD_MEM_TEST)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
endif()

if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
  # TODO Install
else()
  add_library(shm_kernel::memory_manager ALIAS memory_manager)
  set(memory_manager_FOUND TRUE CACHE BOOL)
  set(memory_manager_VERSION ${PROJECT_VERSION} CACHE STRING)
endif()
