cmake_minimum_required(VERSION 3.14)
project(memory_manager
  VERSION 0.7.5
  DESCRIPTION "shm_kernel's memory handler component")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g3 -ggdb")

option(BUILD_TESTING "" ON)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(shared_memory REQUIRED)

include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

write_basic_package_version_file(${CMAKE_PROJECT_NAME}ConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)


if(shared_memory_FOUND)
  message(STATUS "libshared_memory found! ${shared_memory_VERSION}")
else()
  message(ERROR "unalble to find libshared_memory")
endif()

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/bins SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_library(memory_manager STATIC ${SRC})
target_link_libraries(memory_manager PUBLIC fmt::fmt-header-only spdlog::spdlog_header_only shared_memory::shared_memory)
target_compile_features(memory_manager PUBLIC cxx_std_17)
target_compile_options(memory_manager PRIVATE -fPIC)
target_include_directories(memory_manager PUBLIC 
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<INSTALL_INTERFACE:include>")



if (BUILD_TESTING)
  find_package(Catch2 REQUIRED)
  if (NOT Catch2_FOUND)
    message(ERROR "Unable to find Catch2!")
  endif()
  add_executable(Testcase_mem ${CMAKE_CURRENT_SOURCE_DIR}/tests/Testcase_mem.cxx)
  target_link_libraries(Testcase_mem PRIVATE Catch2::Catch2 memory_manager)
  enable_testing()
  add_test(... 
            COMMAND ./Testcase_mem
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIR}/)

endif()

install(TARGETS memory_manager
      EXPORT 
          memory_managerTargets
      RUNTIME 
          DESTINATION ${CMAKE_INSTALL_BINDIR} 
          COMPONENT ${CMAKE_PROJECT_NAME}_Runtime
      LIBRARY 
          DESTINATION ${CMAKE_INSTALL_LIBDIR} 
          COMPONENT ${CMAKE_PROJECT_NAME}_Runtime 
          NAMELINK_COMPONENT ${CMAKE_PROJECT_NAME}_Development
      ARCHIVE 
          DESTINATION ${CMAKE_INSTALL_LIBDIR} 
          COMPONENT ${CMAKE_PROJECT_NAME}_Development
      PUBLIC_HEADER 
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/
)

install(FILES 
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake 
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake
	DESTINATION 
		lib/cmake/memory_manager)

install(DIRECTORY
			${CMAKE_CURRENT_SOURCE_DIR}/include/bins
		DESTINATION 
			include/shm_kernel
		)
install(FILES 
			${CMAKE_CURRENT_SOURCE_DIR}/include/batch.hpp
			${CMAKE_CURRENT_SOURCE_DIR}/include/config.hpp
			${CMAKE_CURRENT_SOURCE_DIR}/include/mem_literals.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/segment.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/mmgr.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/ec.hpp
      ${CMAKE_CURRENT_SOURCE_DIR}/include/except.hpp
		DESTINATION 
			include/memory_manager)

install(EXPORT memory_managerTargets 
	FILE memory_managerTargets.cmake
	NAMESPACE memory_manager::
	DESTINATION lib/cmake/memory_manager)