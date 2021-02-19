cmake_minimum_required(VERSION 3.14)
project(memory_manager
  VERSION 0.7.5
  DESCRIPTION "shm_kernel's memory handler component")

set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -g3 -ggdb")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

option(BUILD_TESTING "" ON)

find_package(fmt REQUIRED)
find_package(spdlog REQUIRED)
find_package(ipc REQUIRED)
include(CMakePackageConfigHelpers)
include(GNUInstallDirs)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src/bins SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

add_library(memory_manager STATIC ${SRC})
target_link_libraries(memory_manager PUBLIC fmt::fmt-header-only spdlog::spdlog_header_only shm_kernel::ipc)
target_compile_features(memory_manager PUBLIC cxx_std_17)
target_compile_options(memory_manager PRIVATE -fPIC)
target_include_directories(memory_manager PUBLIC 
      "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/>"
      "$<INSTALL_INTERFACE:include/shm_kernel>")


if (BUILD_TESTING)
  find_package(Catch2 REQUIRED)
  add_executable(Testcase_mem ${CMAKE_CURRENT_SOURCE_DIR}/tests/Testcase_mem.cxx)
  target_link_libraries(Testcase_mem PRIVATE Catch2::Catch2 memory_manager)
  enable_testing()
  add_test(NAME testcase
            COMMAND ./Testcase_mem
            WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/)
endif()


write_basic_package_version_file(${CMAKE_PROJECT_NAME}ConfigVersion.cmake
    VERSION ${CMAKE_PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion)

install(TARGETS memory_manager
      EXPORT 
          memory_managerTargets
      RUNTIME 
          DESTINATION ${CMAKE_INSTALL_BINDIR} 
      LIBRARY 
          DESTINATION ${CMAKE_INSTALL_LIBDIR} 
      ARCHIVE 
          DESTINATION ${CMAKE_INSTALL_LIBDIR} 
      PUBLIC_HEADER 
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(FILES 
		${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake 
		${CMAKE_CURRENT_SOURCE_DIR}/cmake/${CMAKE_PROJECT_NAME}Config.cmake
	DESTINATION 
		lib/cmake/memory_manager)

install(DIRECTORY
			${CMAKE_CURRENT_SOURCE_DIR}/include/bins
		DESTINATION 
			include/shm_kernel/memory_manager
		)
install(FILES 
			${CMAKE_CURRENT_SOURCE_DIR}/include/batch.hpp
			${CMAKE_CURRENT_SOURCE_DIR}/include/config.hpp
			${CMAKE_CURRENT_SOURCE_DIR}/include/mem_literals.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/segment.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/mmgr.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/ec.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/except.hpp
            ${CMAKE_CURRENT_SOURCE_DIR}/include/smgr.hpp
		DESTINATION 
			include/shm_kernel/memory_manager)

install(EXPORT memory_managerTargets 
	FILE memory_managerTargets.cmake
	NAMESPACE shm_kernel::
	DESTINATION lib/cmake/memory_manager)