cmake_minimum_required(VERSION 3.25)

#[[
 TODO:
 - [ ] add `aletheia_add_test_executable` function for convenience in defining
   and discovering tests when using aletheia as a library
 - [ ] ci (w/ gha-tool)
 - [ ] add benchmark support to aletheia
]]

project(
 aletheia
  VERSION 0.0.0
  LANGUAGES C
  DESCRIPTION "A small and simple C99 unit test suite"
)

#NOTE: only meaningful when building with GNU Make
set(CMAKE_VERBOSE_MAKEFILE TRUE)

#[[setup project base]]
include("${CMAKE_CURRENT_LIST_DIR}/dependencies/cakemake/CMakeLists.txt")

#[[compile options]]
set(ALETHEIA_DEBUG FALSE CACHE BOOL "Enable flags for debug builds")
set(ALETHEIA_MUSL_LIBC FALSE CACHE BOOL "Enable flags for linking against musl libc")
set(ALETHEIA_SANITIZERS FALSE CACHE BOOL "Enable flags for building with sanitizers")
set(ALETHEIA_TESTS FALSE CACHE BOOL "Enable flags for building tests")

#[[add all compiler and linker flags to global lists]]
aletheia_add_default_flags()
aletheia_add_debug_flags(ALETHEIA_DEBUG)
aletheia_add_sanitizer_flags(ALETHEIA_SANITIZERS)
aletheia_add_musl_libc_flags(ALETHEIA_MUSL_LIBC)
aletheia_add_test_flags(ALETHEIA_TESTS)

#[[configure library targets]]
#find sources
set(ALETHEIA_SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/src")
file(
 GLOB_RECURSE
 ALETHEIA_SOURCES
 LIST_DIRECTORIES FALSE
 "${ALETHEIA_SOURCE_DIRECTORY}/*.c"
)

#shared target
function(define_aletheia_shared_library name_prefix dst_prefix)
 set(name "${name_prefix}aletheia")

 add_library("${name}" SHARED EXCLUDE_FROM_ALL)
 target_sources("${name}" PRIVATE ${ALETHEIA_SOURCES})
 target_include_directories("${name}" PUBLIC include)
 target_compile_options("${name}" PRIVATE ${ALETHEIA_COMPILER_FLAGS})
 target_compile_options("${name}" PUBLIC ${ALETHEIA_PUBLIC_COMPILER_FLAGS})
 target_link_options("${name}" PRIVATE ${ALETHEIA_LINKER_FLAGS})
 target_link_options("${name}" PUBLIC ${ALETHEIA_PUBLIC_LINKER_FLAGS})

 set(
  "${dst_prefix}_NAME"
  "${name}"
  PARENT_SCOPE
 )
endfunction()
aletheia_add_source_project(define_aletheia_shared_library)

#static target
function(define_aletheia_static_library name_prefix dst_prefix)
 set(name "${name_prefix}aletheia-static")

 add_library("${name}" STATIC EXCLUDE_FROM_ALL)
 target_sources("${name}" PRIVATE ${ALETHEIA_SOURCES})
 target_include_directories("${name}" PUBLIC include)
 target_compile_options("${name}" PRIVATE ${ALETHEIA_COMPILER_FLAGS})
 target_compile_options("${name}" PUBLIC ${ALETHEIA_PUBLIC_COMPILER_FLAGS})
 target_link_options("${name}" PRIVATE ${ALETHEIA_LINKER_FLAGS})
 target_link_options("${name}" PUBLIC ${ALETHEIA_PUBLIC_LINKER_FLAGS})
 set_target_properties(
  "${name}"
  PROPERTIES
  ARCHIVE_OUTPUT_NAME "${name_prefix}aletheia"
 )

 set(
  "${dst_prefix}_NAME"
  "${name}"
  PARENT_SCOPE
 )
endfunction()
aletheia_add_source_project(define_aletheia_static_library)

#[[configure tests]]
aletheia_add_test_project(
 NAME unit
 SOURCE_DIRECTORY "${PROJECT_SOURCE_DIR}/test/unit"
 INSTRUMENTED_DEPENDENCIES define_aletheia_shared_library
)

foreach(target IN LISTS ALETHEIA_TARGETS)
 get_target_property(
  sources
  "${target}"
  SOURCES
 )
 message(STATUS "SOURCES('${target}'): ${sources}")
 unset(sources)
endforeach()

#[[enforce sanity on all targets for everyone's sake]]
set_target_properties(
 ${ALETHEIA_TARGETS}
 PROPERTIES
  #enforce C99 w/o GNU extensions for aletheia targets
  C_STANDARD 99
  C_EXTENSIONS FALSE
  C_STANDARD_REQUIRED TRUE
  #enable position independent code for aletheia targets
  POSITION_INDEPENDENT_CODE TRUE
  #generate compile commands json for aletheia targets
  EXPORT_COMPILE_COMMANDS TRUE
)
