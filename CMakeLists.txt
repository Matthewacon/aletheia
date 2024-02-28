cmake_minimum_required(VERSION 3.25)

project(
 aletheia
  VERSION 0.0.0
  LANGUAGES C
  DESCRIPTION "A small and simple C99 unit test suite"
)

#[[disallow in-source builds]]
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
 message(
  FATAL_ERROR
  "In-source builds are not permitted."
 )
endif()

#[[ensure compiler is supported]]
#TODO: msvc, ClangCL Intel icc, Intel dpc, tinycc, IBM XL C, qcc
list(
 APPEND ALETHEIA_SUPPORTED_COMPILER_IDS
  Clang
  AppleClang
  GNU
)
#set compiler shorthand identifier so we can easily check later
if(
 CMAKE_C_COMPILER_ID STREQUAL "Clang"
 OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
)
 set(ALETHEIA_COMPILER_ID "Clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
 set(ALETHEIA_COMPILER_ID "GNU")
else()
 message(
  FATAL_ERROR
  "Unsupported compiler: '${CMAKE_C_COMPILER_ID}'\nSupported compilers "
  "include: '${ALETHEIA_SUPPORTED_COMPILER_IDS}'"
 )
endif()
message(STATUS "aletheia: Compiling with '${ALETHEIA_COMPILER_ID}'")

#[[compile options]]
set(ALETHEIA_DEBUG FALSE CACHE BOOL "Enable debug flags for alethia")
set(ALETHEIA_SANITIZERS FALSE CACHE BOOL "Enable sanitizers for aletheia")
set(ALETHEIA_TESTS FALSE CACHE BOOL "Enable validation test suite")

#define library targets so we can configure it downstream
add_library(aletheia SHARED)
add_library(aletheia-static STATIC)

#enforce C99 w/o GNU extensions for aletheia targets
set_property(
 TARGET aletheia aletheia-static
 PROPERTY C_STANDARD 99
)
set_property(
 TARGET aletheia aletheia-static
 PROPERTY C_EXTENSIONS FALSE
)

#enable position independent code for aletheia targets
set_property(
 TARGET aletheia aletheia-static
 PROPERTY POSITION_INDEPENDENT_CODE TRUE
)

#default compiler/linker flags
if(
 ALETHEIA_COMPILER_ID STREQUAL "Clang"
 OR ALETHEIA_COMPILER_ID STREQUAL "GNU"
)
 list(
  APPEND ALETHEIA_COMPILER_FLAGS
   -Werror
   -Wall
   -Wextra
   -pedantic
 )
else()
 #reminder to update when we add support for a new compiler
 message(
  FATAL_ERROR
  "Default compiler flags not implemented for compiler: "
  "${ALETHEIA_COMPILER_ID}"
 )
endif()

#`ALETHEIA_DEBUG` compiler/linker flags
if(ALETHEIA_DEBUG)
 message(STATUS "aletheia: Debug flags enabled")
 if(ALETHEIA_COMPILER_ID STREQUAL "Clang" OR ALETHEIA_COMPILER_ID STREQUAL "GNU")
  #shared clang and gcc flags
  list(
   APPEND ALETHEIA_COMPILER_FLAGS
    -DALETHEIA_DEBUG
    -O0
    -g
    -fno-inline
    -fno-inline-functions
  )
 elseif(ALETHEIA_COMPILER_ID STREQUAL "Clang")
  #clang-specific flags
  list(
   APPEND ALETHEIA_COMPILER_FLAGS
    -fstandalone-debug
  )
 else()
  #reminder to add support for this flag when we add support for a new compiler
  message(
   FATAL_ERROR
   "`ALETHEIA_DEBUG=TRUE` flag not implemented for compiler: "
   "${ALETHEIA_COMPILER_ID}"
  )
 endif()
else()
 message(STATUS "aletheia: Debug flags disabled")
 if(
  ALETHEIA_COMPILER_ID STREQUAL "Clang"
  OR ALETHEIA_COMPILER_ID STREQUAL "GNU"
 )
  #shared clang and gcc flags
  list(
   APPEND ALETHEIA_COMPILER_FLAGS
    -O2
    -flto
  )
  list(
   APPEND ALETHEIA_LINKER_FLAGS
    -flto
  )
 else()
  #reminder to add support for this flag when we add support for a new compiler
  message(
   FATAL_ERROR
   "`ALETHEIA_DEBUG=FALSE` flag not implemented for compiler: "
   "${ALETHEIA_COMPILER_ID}"
  )
 endif()
endif()

#`ALETHEIA_SANITIZERS` compiler/linker flags
if(ALETHEIA_SANITIZERS)
 message(STATUS "aletheia: Sanitizer flags enabled")
 if(
  ALETHEIA_COMPILER_ID STREQUAL "Clang"
  OR ALETHEIA_COMPILER_ID STREQUAL "GNU"
 )
  #shared clang and gcc flags
  list(
   APPEND ALETHEIA_COMPILER_FLAGS
    -fsanitize=address
    -fsanitize=undefined
  )
  list(
   APPEND ALETHEIA_LINKER_FLAGS
    -lasan
    -lubsan
  )
 else()
  #reminder to add support for this flag when we add support for a new compiler
  message(
   FATAL_ERROR
   "`ALETHEIA_SANITIZERS=TRUE` flag not implemented for compiler: "
   "${ALETHEIA_COMPILER_ID}"
  )
 endif()
else()
 #NOTE: no extra flags when sanitizers disabled
 message(STATUS "aletheia: Sanitizer flags disabled")
endif()

#[[configure library targets]]
#find sources
file(
 GLOB_RECURSE
 ALETHEIA_SOURCES
 LIST_DIRECTORIES FALSE
 "src/*.c"
)

#shared target
target_sources(aletheia PRIVATE ${ALETHEIA_SOURCES})
target_include_directories(aletheia PUBLIC include)
target_compile_options(aletheia PRIVATE ${ALETHEIA_COMPILER_FLAGS})
target_link_options(aletheia PRIVATE ${ALETHEIA_LINKER_FLAGS})

#static target
target_sources(aletheia-static PRIVATE ${ALETHEIA_SOURCES})
target_include_directories(aletheia-static PUBLIC include)
target_compile_options(aletheia-static PRIVATE ${ALETHEIA_COMPILER_FLAGS})
target_link_options(aletheia-static PRIVATE ${ALETHEIA_LINKER_FLAGS})
set_target_properties(
 aletheia-static
 PROPERTIES
 ARCHIVE_OUTPUT_NAME "aletheia"
)

#[[tests]]
#`ALETHEIA_TESTS` flag
if(NOT ALETHEIA_TESTS)
 message(STATUS "aletheia: Tests disabled")
 return()
else()
 message(STATUS "aletheia: Tests enabled")
endif()

#TODO: coverage flags

enable_testing()

#[[unit test targets]]
#collect test sources
set(ALETHEIA_TEST_UNIT_DIR "${PROJECT_SOURCE_DIR}/test/unit")
set(ALETHEIA_TEST_UNIT_INCLUDE_DIR "${ALETHEIA_TEST_UNIT_DIR}/include")
set(ALETHEIA_TEST_UNIT_SOURCE_DIR "${ALETHEIA_TEST_UNIT_DIR}/src")
set(ALETHEIA_TEST_UNIT_OUTPUT_DIR "${PROJECT_BINARY_DIR}/test/unit")
file(
 GLOB_RECURSE
 TEST_SOURCES
 LIST_DIRECTORIES FALSE
 "${ALETHEIA_TEST_UNIT_SOURCE_DIR}/*.c"
)

#configure test targets
foreach(test_src IN LISTS TEST_SOURCES)
 #transform test executable name
 file(TO_CMAKE_PATH "${test_src}" test_src_path)
 cmake_path(
  RELATIVE_PATH test_src_path
  BASE_DIRECTORY "${ALETHEIA_TEST_UNIT_SOURCE_DIR}"
  OUTPUT_VARIABLE test_src_path
 )
 string(
  REPLACE
   "/"
   "."
   test_name
  "${test_src_path}"
 )
 string(LENGTH "${test_name}" test_name_length)
 math(EXPR test_name_length "${test_name_length} - 2")
 string(
  SUBSTRING
   "${test_name}"
   0
   ${test_name_length}
   test_name
 )
 set(test_name "unit.${test_name}")
 unset(test_src_path)

 #add executable target
 add_executable("${test_name}" "${test_src}")
 target_include_directories("${test_name}" PUBLIC "${ALETHEIA_TEST_UNIT_INCLUDE_DIR}")
 target_compile_options("${test_name}" PUBLIC ${ALETHEIA_COMPILER_FLAGS})
 target_link_options("${test_name}" PUBLIC ${ALETHEIA_LINKER_FLAGS})
 target_link_libraries("${test_name}" PUBLIC aletheia-static)
 set_property(
  TARGET "${test_name}"
  PROPERTY POSITION_INDEPENDENT_CODE TRUE
 )

 #set output directory
 set_target_properties(
  "${test_name}"
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${ALETHEIA_TEST_UNIT_OUTPUT_DIR}"
 )

 #add test target
 add_test(
  NAME "${test_name}"
  COMMAND "${test_name}"
  WORKING_DIRECTORY "${ALETHEIA_TEST_UNIT_OUTPUT_DIR}"
 )
endforeach()
unset(test_name_length)
unset(test_name)
unset(test_src_path)

#TODO: [[coverage targets]]
#TODO: add target to combine test coverage data
#TODO: add test coverage combination target to test target
#TODO: add test target to default target list
