cmake_minimum_required(VERSION 3.25)

include_guard(GLOBAL)

#TODO: include kitchen-sink
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/projects/kitchen-sink")

#include aletheia
add_subdirectory("${CMAKE_CURRENT_LIST_DIR}/projects/aletheia")
