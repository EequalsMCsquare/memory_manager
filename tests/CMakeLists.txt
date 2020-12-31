cmake_minimum_required(VERSION 3.14)

find_package(Catch2)

if(NOT Catch2_FOUND)
	include(FetchContent)
	FetchContent_Declare(
		Catch2 
		GIT_REPOSITORY https://github.com.cnpmjs.org/catchorg/Catch2.git
		GIT_TAG        v2.13.4
    )
else()
  message(STATUS "Catch found!")
endif()

add_executable(Test_static_bin ${CMAKE_CURRENT_SOURCE_DIR}/Test_static_bin.cxx)
target_link_libraries(Test_static_bin PRIVATE memory_manager Catch2::Catch2)
