cmake_minimum_required(VERSION 3.25)

include_guard(GLOBAL)

string(TOUPPER "${PROJECT_NAME}" VARIABLE_PREFIX)
set(VARIABLE_PREFIX "${VARIABLE_PREFIX}" PARENT_SCOPE)
string(TOLOWER "${PROJECT_NAME}_" FUNCTION_PREFIX)
set(FUNCTION_PREFIX "${FUNCTION_PREFIX}" PARENT_SCOPE)

#[[disallow in-source builds]]
if("${PROJECT_SOURCE_DIR}" STREQUAL "${PROJECT_BINARY_DIR}")
 message(FATAL_ERROR "In-source builds are not permitted")
endif()

#[[ensure compiler is supported]]
#TODO: msvc, ClangCL, Intel icc, Intel dpc, IBM XL C, qcc
list(
 APPEND "${VARIABLE_PREFIX}_SUPPORTED_COMPILER_IDS"
  Clang
  AppleClang
  GNU
  TinyCC
)
#set compiler shorthand identifier so we can easily check later
if(
 CMAKE_C_COMPILER_ID STREQUAL "Clang"
 OR CMAKE_C_COMPILER_ID STREQUAL "AppleClang"
)
 set("${VARIABLE_PREFIX}_COMPILER_ID" "Clang")
elseif(CMAKE_C_COMPILER_ID STREQUAL "GNU")
 set("${VARIABLE_PREFIX}_COMPILER_ID" "GNU")
elseif(CMAKE_C_COMPILER_ID STREQUAL "TinyCC")
 set("${VARIABLE_PREFIX}_COMPILER_ID" "TinyCC")
else()
 message(
  FATAL_ERROR
  "Unsupported compiler: '${CMAKE_C_COMPILER_ID}'\nSupported compilers "
  "include: '${${VARIABLE_PREFIX}_SUPPORTED_COMPILER_IDS}'"
 )
endif()
message(STATUS "${PROJECT_NAME}: Compiling with '${${VARIABLE_PREFIX}_COMPILER_ID}'")

#[[
 TODO: create project-specific top-level targets

  - these are platform-specific, move to platform-specific directories
]]
#[[
 top-level clean target

 custom user-specified clean targets can add themselves to this target as
 dependencies and when this target is built, all user-specified clean targets
 will be built and the auto-generated 'clean' target will be invoked
]]
add_custom_target(
 clean-all
 WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target clean
 VERBATIM
)

#[[
 top-level target to build all tests

 tests that are declared as part of the project are added as dependencies of
 this target, such that we can build this target to build all tests

 NOTE: performs a no-op command by default, in case no user-specified coverage
 targets are added as dependencies of this target
]]
add_custom_target(
 build-test-all
 COMMAND /usr/bin/bash -c "exit 0"
 VERBATIM
)

#[[
 top-level target to build and run all tests

 NOTE: profile and coverage data is not cleaned between runs
]]
add_custom_target(
 test-all
 WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target build-test-all
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target test
 VERBATIM
)

#[[
 top-level test coverage target

 NOTE: performs a no-op command by default, in case no user-specified coverage
 targets are added as dependencies of this target
]]
add_custom_target(
 coverage
 WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
 COMMAND /usr/bin/bash -c "exit 0"
 VERBATIM
)

#[[
 top-level test coverage clean up target

 NOTE: performs a no-op command by default, in case no user-specified coverage
 targets are added as dependencies of this target
]]
add_custom_target(
 clean-coverage
 WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
 COMMAND /usr/bin/bash -c "exit 0"
 VERBATIM
)
add_dependencies(clean-all clean-coverage)

#TODO: top-level benchmark target

#[[
 top-level CI target

 runs:
  1. `clean-coverage`: cleans all prior test profile data and test reports
  2. `test-all`: runs all tests
  3. `coverage`: generates test coverage reports using test profile data
  4. `benchmark`: TODO
]]
add_custom_target(
 ci
 WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target clean-coverage
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target test-all
 COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target coverage
 #TODO: add benchmarks to ci
 #COMMAND ${CMAKE_COMMAND} --build "${PROJECT_BINARY_DIR}" --target benchmark
 VERBATIM
)

#[[
 include compiler directory

 functions used below are defined, for their respective platform / compiler
 combination, in the subdirectories included in this switch block
]]
if("${${VARIABLE_PREFIX}_COMPILER_ID}" STREQUAL "Clang")
 add_subdirectory(clang)
elseif("${${VARIABLE_PREFIX}_COMPILER_ID}" STREQUAL "GNU")
 add_subdirectory(gcc)
elseif("${${VARIABLE_PREFIX}_COMPILER_ID}" STREQUAL "TinyCC")
 add_subdirectory(tcc)
else()
 #reminder to update when we add support for a new compiler
 message(
  FATAL_ERROR
  "Support for compiler '${${VARIABLE_PREFIX}_COMPILER_ID}' is umimplemented!"
 )
endif()

#[[
 utility function for declaring a common test target

 adds two new targets: `${target_name}` and `${target_name}-static` that can
 be used wherever

 usage: add_common_test_project(<name> <path>)
]]
function(
 "${FUNCTION_PREFIX}add_common_test_project"
  target_name
  project_directory
)
 #ensure we were given a target name
 string(LENGTH "${target_name}" target_name_length)
 if(${target_name_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing target name!"
  )
 endif()
 unset(target_name_length)

 #ensure we were given a project directory
 string(LENGTH "${project_directory}" project_directory_length)
 if(${project_directory_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing project directory!"
  )
 endif()
 unset(project_directory_length)

 #project compile and link flag variables
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 set(linker_flags_var "${VARIABLE_PREFIX}_LINKER_FLAGS")

 #find sources
 set(source_directory "${project_directory}/src")
 set(include_directory "${project_directory}/include")
 file(
  GLOB_RECURSE
  TEST_COMMON_SOURCES
  LIST_DIRECTORIES FALSE
  "${source_directory}/*.c"
 )

 #shared target
 add_library("${target_name}" SHARED)
 target_sources("${target_name}" ${TEST_COMMON_SOURCES})
 target_include_directories("${target_name}" PUBLIC "${include_directory}")
 target_compile_options("${target_name}" PRIVATE ${${compiler_flags_var}})
 target_link_options("${target_name}" PRIVATE ${${linker_flags_var}})

 #static target
 add_library("${target_name}-static" STATIC)
 target_sources("${target_name}-static" ${TEST_COMMON_SOURCES})
 target_include_directories("${target_name}" PUBLIC "${include_directory}")
 target_compile_options("${target_name}" PRIVATE ${${compiler_flags_var}})
 target_link_options("${target_name}" PRIVATE ${${linker_flags_var}})

 #enable PIC for both targets
 set_property(
  TARGET "${target_name}" "${target_name}-static"
  PROPERTY POSITION_INDEPENDENT_CODE TRUE
 )

 #append common test library targets to global target list for this project
 list(
  APPEND "${VARIABLE_PREFIX}_TARGETS"
   "${target_name}"
   "${target_name}-static"
 )
 set(
  "${VARIABLE_PREFIX}_TARGETS"
  "${${VARIABLE_PREFIX}_TARGETS}"
  PARENT_SCOPE
 )
endfunction()

#[[
 usage: add_source_project(callback)

 Expects a callback with the format
 `function(define_my_target name_prefix dst_prefix)`; the provided callback
 must:
  - include the `name_prefix` parameter in the name during the declaration
    of the target
  - provide the target name in the destination variable specified by
    `${dst_prefix}_NAME`
  - provide the target source directory in the destination variable specified
    by `${dst_prefix}_SOURCE_DIRECTORY`

 Example:
  function(define_my_target name_prefix dst_prefix)
   set(name "${name_prefix}my-target")
   set(source_dir "${PROJECT_SOURCE_DIR}/src")

   add_library("${name}" SHARED)
   add_sources("${name}" PUBLIC "${source_dir}/lib.c")
   #...

   set("${dst_prefix}_NAME" "${name}" PARENT_SCOPE)
   set("${dst_prefix}_SOURCE_DIRECTORY" "${source_dir}" PARENT_SCOPE)
  endfunction()
]]
function("${FUNCTION_PREFIX}add_source_project" callback)
 #ensure we were provided with a callback
 string(LENGTH "${callback}" callback_LENGTH)
 if(${callback_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing callback function name!")
 endif()
 unset(callback_LENGTH)

 #invoke callback with empty prefix to create global default importable target
 cmake_language(CALL "${callback}" "" asp)

 #ensure the callback provided us with the resulting target name
 string(LENGTH "${asp_NAME}" asp_NAME_LENGTH)
 if(${asp_NAME_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Target name not set in 'asp_NAME' variable!")
 endif()
 unset(asp_NAME_LENGTH)

 #append target name to global list of targets
 set(target_list_var "${VARIABLE_PREFIX}_TARGETS")
 list(APPEND "${target_list_var}" "${asp_NAME}")
 set("${target_list_var}" "${${target_list_var}}" PARENT_SCOPE)
endfunction()

#[[
 utility function for adding test projects

 usage: add_test_project(
  NAME <name>
  SOURCE_DIRECTORY <path>
  INSTRUMENTED_DEPENDENCIES [<callbacks>...]
  LINK_LIBRARIES [<libraries>...]
 )
]]
function("${FUNCTION_PREFIX}add_test_project")
 #parse arguments
 cmake_parse_arguments(
  atp
  ""
  "NAME;SOURCE_DIRECTORY"
  "INSTRUMENTED_DEPENDENCIES;LINK_LIBRARIES"
  ${ARGN}
 )

 #[[validate arguments]]
 #ensure we were given a test project name
 string(LENGTH "${atp_NAME}" atp_NAME_length)
 if(${atp_NAME_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing test project name!"
  )
 endif()
 unset(atp_NAME_length)

 #ensure we were given a test project directory
 string(LENGTH "${atp_SOURCE_DIRECTORY}" atp_SOURCE_DIRECTORY_length)
 if(${atp_SOURCE_DIRECTORY_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing test project directory!"
  )
 endif()
 unset(atp_SOURCE_DIRECTORY_length)

 #[[define instrumented dependencies, if specified]]
 set(global_target_list_var "${VARIABLE_PREFIX}_TARGETS")
 if(DEFINED atp_INSTRUMENTED_DEPENDENCIES)
  foreach(dependency IN LISTS atp_INSTRUMENTED_DEPENDENCIES)
   #invoke callback with test name prefix to create test-specific target
   cmake_language(CALL "${dependency}" "test-${atp_NAME}-" instrumented)

   #validate `instrumented_NAME`
   string(LENGTH "${instrumented_NAME}" instrumented_NAME_LENGTH)
   if(${instrumented_NAME_LENGTH} EQUAL 0)
    message(FATAL_ERROR "Target name not set in 'instrumented_NAME' variable!")
   endif()
   unset(instrumented_NAME_LENGTH)

   #add target name to list of instrumented targets for use below
   list(APPEND instrumented_dependencies "${instrumented_NAME}")

   #add target name to global list of targets
   list(APPEND "${global_target_list_var}" "${instrumented_NAME}")
   set(
    "${global_target_list_var}"
    "${${global_target_list_var}}"
    PARENT_SCOPE
   )
   unset(instrumented_NAME)
  endforeach()
 endif()

 #find sources
 set(source_directory "${atp_SOURCE_DIRECTORY}/src")
 set(include_directory "${atp_SOURCE_DIRECTORY}/include")
 set(test_output_directory "${PROJECT_BINARY_DIR}/test/${atp_NAME}")
 set(test_project_output_directory "${test_output_directory}/build")
 set(test_profile_output_directory "${test_output_directory}/profile")
 set(test_report_output_directory "${test_output_directory}/report")
 file(
  GLOB_RECURSE
  TEST_SOURCES
  LIST_DIRECTORIES FALSE
  "${source_directory}/*.c"
 )

 #configure tests
 foreach(test_src IN LISTS TEST_SOURCES)
  message(STATUS "processing test src: ${test_src}")
  #transform test executable name
  file(TO_CMAKE_PATH "${test_src}" test_src_path)
  cmake_path(
   RELATIVE_PATH test_src_path
   BASE_DIRECTORY "${source_directory}"
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
  set(test_name "${test_name}")
  unset(test_src_path)

  #call platform/compiler-specific `add_test(...)` function
  cmake_language(
   CALL "${FUNCTION_PREFIX}add_test"
    TEST_NAME "${test_name}"
    TEST_PROJECT_NAME "${atp_NAME}"
    TEST_SOURCES "${test_src}"
    TEST_OUTPUT_DIRECTORY "${test_project_output_directory}"
    TEST_PROFILE_OUTPUT_DIRECTORY "${test_profile_output_directory}"
    TEST_REPORT_OUTPUT_DIRECTORY "${test_report_output_directory}"
    TEST_INCLUDE_DIRECTORIES "${include_directory}"
    TEST_LINK_INSTRUMENTED_DEPENDENCIES ${instrumented_dependencies}
    TEST_LINK_LIBRARIES ${atp_LINK_LIBRARIES}
  )
  #NOTE: have to propagate `${VARIABLE_PREFIX}_TARGETS` due to scope nesting
  set(
   "${global_target_list_var}"
   "${${global_target_list_var}}"
   PARENT_SCOPE
  )
 endforeach()

 #call platform/compiler-specific `add_coverage_targets(...)` function
 cmake_language(
  CALL "${FUNCTION_PREFIX}add_coverage_targets"
   TEST_PROJECT_NAME "${atp_NAME}"
   TEST_PROFILE_OUTPUT_DIRECTORY "${test_profile_output_directory}"
   TEST_REPORT_OUTPUT_DIRECTORY "${test_report_output_directory}"
   TEST_LINK_INSTRUMENTED_DEPENDENCIES ${instrumented_dependencies}
 )
 #NOTE: have to propagate `${VARIABLE_PREFIX}_TARGETS` due to scope nesting
 set(
  "${global_target_list_var}"
  "${${global_target_list_var}}"
  PARENT_SCOPE
 )
endfunction()
