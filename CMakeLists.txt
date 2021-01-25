cmake_minimum_required(VERSION 3.14)
project(memory_manager
  VERSION 0.4.0
  DESCRIPTION "shm_kernel's memory handler component")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

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
target_link_libraries(memory_manager PUBLIC fmt::fmt spdlog::spdlog shm_kernel::shared_memory)
target_include_directories(memory_manager PUBLIC 
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
      "$<INSTALL_INTERFACE:include/shm_kernel>")


option(BUILD_MEM_TEST "build with testcases" ON)

if (BUILD_MEM_TEST)
  add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/tests)
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
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/shm_kernel
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
      ${CMAKE_CURRENT_SOURCE_DIR}/include/memmgr.hpp
		DESTINATION 
			include/shm_kernel)

install(EXPORT memory_managerTargets 
	FILE memory_managerTargets.cmake
	NAMESPACE shm_kernel::
	DESTINATION lib/cmake/memory_manager)