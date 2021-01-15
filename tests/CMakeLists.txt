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

function(ADD_TESTBIN Testcase_name)
	message(STATUS "Register testcase binary: ${Testcase_name}. Compiled from ${Testcase_name}.cxx")
	add_executable(${Testcase_name} ${CMAKE_CURRENT_SOURCE_DIR}/${Testcase_name}.cxx)
	target_link_libraries(${Testcase_name} PRIVATE memory_manager Catch2::Catch2)
endfunction()

ADD_TESTBIN(Test_static_bin)
ADD_TESTBIN(Test_instant_bin)
ADD_TESTBIN(Test_batch)
ADD_TESTBIN(Test_arena)
