cmake_minimum_required(VERSION 3.10)

project(dokugen)

# add CMakeModules to path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/CMakeModules")

# set output folder to /bin/
get_filename_component(CONFIG_NAME "${CMAKE_BINARY_DIR}" NAME)
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}")
set(LIBRARY_OUTPUT_PATH "${CMAKE_SOURCE_DIR}/bin/${CONFIG_NAME}")

# packages
find_package(XO)

# contrib include paths
include_directories(${CMAKE_SOURCE_DIR}/contrib/tclap-1.2.1/include)
include_directories(${CMAKE_SOURCE_DIR}/contrib/rapidxml-1.13)

# projects
add_subdirectory(submodules/xo)
add_subdirectory(src)
