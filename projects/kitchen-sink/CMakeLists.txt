cmake_minimum_required(VERSION 3.25)

include_guard(GLOBAL)

project(
 kitchen-sink
  VERSION 0.0.0
  LANGUAGES C
  DESCRIPTION "A C99 library with shared utilities for my projects"
)

#NOTE: only meaningful when building with GNU Make
set(CMAKE_VERBOSE_MAKEFILE TRUE)

#[[setup project base]]
include("${CMAKE_CURRENT_LIST_DIR}/../../dependencies/cakemake/CMakeLists.txt")

#[[compile options]]
set(KITCHEN_SINK_DEBUG FALSE CACHE BOOL "Enable flags for debug builds")
set(KITCHEN_SINK_MUSL_LIBC FALSE CACHE BOOL "Enable flags for linking against musl libc")
set(KITCHEN_SINK_SANITIZERS FALSE CACHE BOOL "Enable flags for building with sanitizers")
set(KITCHEN_SINK_TESTS FALSE CACHE BOOL "Enable flags for building tests")

#[[add all compiler and linker flags to global lists]]
kitchen_sink_add_default_flags()
kitchen_sink_add_debug_flags(KITCHEN_SINK_DEBUG)
kitchen_sink_add_sanitizer_flags(KITCHEN_SINK_SANITIZERS)
kitchen_sink_add_musl_libc_flags(KITCHEN_SINK_MUSL_LIBC)
kitchen_sink_add_test_flags(KITCHEN_SINK_TESTS)

#[[configure library target]]
#find sources
set(KITCHEN_SINK_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/src")
set(KITCHEN_SINK_INCLUDE_DIRECTORY "${PROJECT_SOURCE_DIR}/include")
file(
 GLOB_RECURSE
 KITCHEN_SINK_SOURCES
 LIST_DIRECTORIES FALSE
 "${KITCHEN_SINK_SOURCE_DIRECTORY}/*.c"
)

#library target
function(define_kitchen_sink_library name_prefix dst_prefix)
 set(name "${name_prefix}kitchen-sink")

 add_library("${name}")
 target_sources("${name}" PRIVATE ${KITCHEN_SINK_SOURCES})
 target_include_directories("${name}" PUBLIC "${KITCHEN_SINK_INCLUDE_DIRECTORY}")
 target_compile_options("${name}" PRIVATE ${KITCHEN_SINK_COMPILER_FLAGS})
 target_compile_options("${name}" PUBLIC ${KITCHEN_SINK_PUBLIC_COMPILER_FLAGS})
 target_link_options("${name}" PRIVATE ${KITCHEN_SINK_LINKER_FLAGS})
 target_link_options("${name}" PUBLIC ${KITCHEN_SINK_PUBLIC_LINKER_FLAGS})

 set(
  "${dst_prefix}_NAME"
  "${name}"
  PARENT_SCOPE
 )
endfunction()
kitchen_sink_add_source_project(define_kitchen_sink_library)

#[[configure tests]]
kitchen_sink_add_test_project(
 NAME unit
 SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/test/unit"
 INSTRUMENTED_DEPENDENCIES define_kitchen_sink_library
)

#[[enforce sanity on all targets for everyone's sake]]
set_target_properties(
 ${KITCHEN_SINK_TARGETS}
 PROPERTIES
  #enforce C99 w/o GNU extensions for kitchen-sink targets
  C_STANDARD 99
  C_EXTENSIONS FALSE
  C_STANDARD_REQUIRED TRUE
  #enable position independent code for kitchen-sink targets
  POSITION_INDEPENDENT_CODE TRUE
  #generate compile commands json for kitchen-sink targets
  EXPORT_COMPILE_COMMANDS TRUE
)
