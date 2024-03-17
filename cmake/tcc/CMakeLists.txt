include_guard(GLOBAL)

#[[
 this file contains the implementations of all build functions used in the
 top-level `CMakeLists.txt` for clang
]]

#[[
 adds the default build flags to the `${PROJECT_NAME}_COMPILER_FLAGS` and
 `${PROJECT_NAME}_LINKER_FLAGS` lists

 usage: add_default_flags()
]]
function("${FUNCTION_PREFIX}add_default_flags")
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 list(
  APPEND "${compiler_flags_var}"
   -Werror
   -Wall
   -Wextra
   -pedantic
 )

 #propagate compiler flags to global scope
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 adds debug or non-debug flags, based on the value of `${${debug_flag_var}}`,
 to the `${PROJECT_NAME}_COMPILER_FLAGS` and `${PROJECT_NAME}_LINKER_FLAGS`
 lists

 usage: add_debug_flags(<flag variable>)
]]
function("${FUNCTION_PREFIX}add_debug_flags" debug_flag_var)
 #ensure we were given a debug flag name
 string(LENGTH "${debug_flag_var}" debug_flag_var_length)
 if(${debug_flag_var_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing debug flag name!"
  )
 endif()
 unset(debug_flag_var_length)

 #ensure the debug flag provided actually exists
 if(NOT DEFINED "${debug_flag_var}")
  message(
   FATAL_ERROR
   "'${debug_flag_var}' variable does not exist! Did you set up the flag "
   "before calling this function?"
  )
 endif()

 #add debug/non-debug flags
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 if(${${debug_flag_var}})
  message(STATUS "${PROJECT_NAME}: Debug flags enabled")
  list(
   APPEND "${compiler_flags_var}"
    -D${debug_flag_var}
    -g
  )
 else()
  message(STATUS "${PROJECT_NAME}: Debug flags disabld")
 endif()

 #propagate compiler flags to global scope
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 currently does nothing on tcc as there is no sanitizer support for tcc

 usage: add_sanitizer_flags(<flag variable>)
]]
function("${FUNCTION_PREFIX}add_sanitizer_flags" sanitizer_flag_var)
 message(STATUS "${PROJECT_NAME}: Sanitizer flags are not supported on tcc!")

 #ensure we were given a sanitizer flag name
 string(LENGTH "${sanitizer_flag_var}" sanitizer_flag_var_length)
 if(${sanitizer_flag_var_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing sanitizer flag name!"
  )
 endif()
 unset(sanitizer_flag_var_length)

 #ensure the sanitizer flag provided actually exists
 if(NOT DEFINED "${sanitizer_flag_var}")
  message(
   FATAL_ERROR
   "'${sanitizer_flag_var}' variable does not exist! Did you set up the flag "
   "before calling this function?"
  )
 endif()

 #add sanitizer/non-sanitizer flags
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 if(${${sanitizer_flag_var}})
  message(STATUS "${PROJECT_NAME}: Sanitizer flags enabled")
  list(
   APPEND "${compiler_flags_var}"
    -D${sanitizer_flag_var}
  )
 else()
  message(STATUS "${PROJECT_NAME}: Sanitizer flags disabled")
 endif()

 #propagate compiler flags to global scope
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 adds test flags, based on the value of `${${test_flag_var}}`,
 to the `${PROJECT_NAME}_COMPILER_FLAGS` and `${PROJECT_NAME}_LINKER_FLAGS`
 lists

 usage: add_test_flags(<flag variable>)
]]
function("${FUNCTION_PREFIX}add_test_flags" test_flag_var)
 #ensure we were given a test flag name
 string(LENGTH "${test_flag_var}" test_flag_var_length)
 if(${test_flag_var_length} EQUAL 0)
  message(
   FATAL_ERROR
   "Missing test flag name!"
  )
 endif()
 unset(test_flag_var_length)

 #ensure the test flag provided actually exists
 if(NOT DEFINED "${test_flag_var}")
  message(
   FATAL_ERROR
   "'${test_flag_var}' variable does not exist! Did you set up the flag "
   "before calling this function?"
  )
 endif()

 ##add test/non-test flags
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 if(${${test_flag_var}})
  message(STATUS "${PROJECT_NAME}: Tests enabled")
  set("${VARIABLE_PREFIX}_TESTS_ENABLED" TRUE PARENT_SCOPE)

  list(
   APPEND "${compiler_flags_var}"
    -D${test_flag_var}
  )
 else()
  message(STATUS "${PROJECT_NAME}: Tests disabled")
  set("${VARIABLE_PREFIX}_TESTS_ENABLED" FALSE PARENT_SCOPE)
 endif()

 #propagate compiler flags to global scope
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )

endfunction()

#[[
  adds a test using the compiler and linker flags specified in the
 `${PROJECT_NAME}_COMPILER_FLAGS` and `${PROJECT_NAME}_LINKER_FLAGS` lists

 usage: add_test(
  TEST_NAME <test name>
  TEST_OUTPUT_DIRECTORY <path>
  TEST_SOURCES <path> [<path>...]
  TEST_INCLUDE_DIRECTORIES [<path>...]
  TEST_LINK_INSTRUMENTED_DEPENDENCIES [<target name>...]
  TEST_LINK_LIBRARIES [<target name>...]
 )
]]
function("${FUNCTION_PREFIX}add_test")
 #parse arguments
 cmake_parse_arguments(
  at
  ""
  "TEST_NAME;TEST_PROJECT_NAME;TEST_OUTPUT_DIRECTORY;TEST_PROFILE_OUTPUT_DIRECTORY;TEST_REPORT_OUTPUT_DIRECTORY"
  "TEST_SOURCES;TEST_INCLUDE_DIRECTORIES;TEST_LINK_INSTRUMENTED_DEPENDENCIES;TEST_LINK_LIBRARIES"
  ${ARGN}
 )

 #[[validate arguments]]
 #ensure we were provided with a valid test name
 string(LENGTH "${at_TEST_NAME}" at_TEST_NAME_LENGTH)
 if(${at_TEST_NAME_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test name!")
 endif()
 unset(at_TEST_NAME_LENGTH)

 #ensure we were provided with a valid test project name
 string(LENGTH "${at_TEST_PROJECT_NAME}" at_TEST_PROJECT_NAME_LENGTH)
 if(${at_TEST_PROJECT_NAME_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test project name!")
 endif()
 unset(at_TEST_PROJECT_NAME_LENGTH)

 #ensure we were provided with a test output directory
 string(LENGTH "${at_TEST_OUTPUT_DIRECTORY}" at_TEST_OUTPUT_DIRECTORY_LENGTH)
 if(${at_TEST_OUTPUT_DIRECTORY_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test output directory!")
 endif()
 unset(at_TEST_OUTPUT_DIRECTORY_LENGTH)

 #ensure we were provided with a test profiling output directory
 string(LENGTH "${at_TEST_PROFILE_OUTPUT_DIRECTORY}" at_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH)
 if(${at_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test profile output directory!")
 endif()
 unset(at_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH)

 #ensure we were provided with a test report output directory
 string(LENGTH "${at_TEST_REPORT_OUTPUT_DIRECTORY}" at_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH)
 if(${at_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test report output directory!")
 endif()
 unset(at_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH)

 #ensure we were provided with at least one test source file
 list(LENGTH at_TEST_SOURCES at_TEST_SOURCES_LENGTH)
 if(${at_TEST_SOURCES_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing at least 1 source file for test!")
 endif()
 unset(at_TEST_SOURCES_LENGTH)

 #determine whether we need to specify include dierctores
 list(LENGTH at_TEST_INCLUDE_DIRECTORIES at_TEST_INCLUDE_DIRECTORIES_LENGTH)
 if(${at_TEST_INCLUDE_DIRECTORIES_LENGTH} EQUAL 0)
  set(has_includes FALSE)
 else()
  set(has_includes TRUE)
 endif()
 unset(at_TEST_INCLUDE_DIRECTORIES_LENGTH)

 #determine whether we need to specify instrumented link targets
 list(
  LENGTH
   at_TEST_LINK_INSTRUMENTED_DEPENDENCIES
   at_TEST_LINK_INSTRUMENTED_DEPENDENCIES_LENGTH
 )
 if(${at_TEST_LINK_INSTRUMENTED_DEPENDENCIES_LENGTH} EQUAL 0)
  set(has_instrumented_dependencies FALSE)
 else()
  set(has_instrumented_dependencies TRUE)
 endif()

 #determine whether we need to specify library link targets
 list(LENGTH at_TEST_LINK_LIBRARIES at_TEST_LINK_LIBRARIES_LENGTH)
 if(${at_TEST_LINK_LIBRARIES_LENGTH} EQUAL 0)
  set(has_libraries FALSE)
 else()
  set(has_libraries TRUE)
 endif()
 unset(at_TEST_LINK_LIBRARIES_LENGTH)

 #skip adding test if tests are not enabled for this build
 if(
  NOT DEFINED "${VARIABLE_PREFIX}_TESTS_ENABLED"
  OR NOT ${${VARIABLE_PREFIX}_TESTS_ENABLED}
 )
  return()
 endif()

 #[[
  NOTE: tcc does not support binary instrumentation, so we do not need to
  modify instrumented link dependencies
 ]]

 #[[set up test executable]]
 #project compile and link flag variables
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 set(linker_flags_var "${VARIABLE_PREFIX}_LINKER_FLAGS")

 #add test executable
 add_executable("${at_TEST_NAME}" ${at_TEST_SOURCES})
 target_compile_options("${at_TEST_NAME}" PUBLIC ${${compiler_flags_var}})
 target_link_options("${at_TEST_NAME}" PUBLIC ${${linker_flags_var}})

 #add test executable to top-level build-test-all target
 add_dependencies(build-test-all "${at_TEST_NAME}")

 #add include directories, if specified
 if(has_includes)
  target_include_directories(
   "${at_TEST_NAME}"
   PUBLIC
    ${at_TEST_INCLUDE_DIRECTOIRES}
  )
 endif()
 unset(has_includes)

 #add libraries, if specified
 if(has_libraries)
  target_link_libraries(
   "${at_TEST_NAME}"
   PUBLIC
    ${at_TEST_LINK_LIBRARIES}
  )
 endif()
 unset(has_libraries)

 #add instrumented dependencies, if specified
 if(has_instrumented_dependencies)
  target_link_libraries(
   "${at_TEST_NAME}"
   PUBLIC
    ${at_TEST_LINK_INSTRUMENTED_DEPENDENCIES}
  )
 endif()
 unset(has_instrumented_dependencies)

 #set test executable output directory
 set_target_properties(
  "${at_TEST_NAME}"
  PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY "${at_TEST_OUTPUT_DIRECTORY}"
 )

 #ensure test executable is PIE
 set_property(
  TARGET "${at_TEST_NAME}"
  PROPERTY POSITION_INDEPENDENT_CODE TRUE
 )

 #add test
 add_test(
  NAME "${at_TEST_NAME}"
  COMMAND "${at_TEST_NAME}"
  WORKING_DIRECTORY "${at_TEST_OUTPUT_DIRECTORY}"
 )

 #set LD_LIBRARY_PATH for tcc-compiled tests
 set_property(
  TEST "${at_TEST_NAME}"
  PROPERTY
   ENVIRONMENT "LD_LIBRARY_PATH=${PROJECT_BINARY_DIR}"
 )

 #add custom commands to generate the report output directory
 add_custom_command(
  OUTPUT "${at_TEST_REPORT_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash ARGS -c "echo 'Creating \"${at_TEST_PROJECT_NAME}\" test project report output directory...'; mkdir -p '${at_TEST_REPORT_OUTPUT_DIRECTORY}'"
  VERBATIM
 )
 set(target_name_prefix "test-${at_TEST_NAME}")
 set(
  create_dirs_target_name
  "${target_name_prefix}-create-profile-and-report-directories"
 )
 add_custom_target(
  "${create_dirs_target_name}"
  DEPENDS "${at_TEST_REPORT_OUTPUT_DIRECTORY}"
 )
 add_dependencies(test-all "${create_dirs_target_name}")

 #append test to global target list for this project
 list(
  APPEND "${VARIABLE_PREFIX}_TARGETS"
   "${at_TEST_NAME}"
 )
 set(
  "${VARIABLE_PREFIX}_TARGETS"
  "${${VARIABLE_PREFIX}_TARGETS}"
  PARENT_SCOPE
 )
endfunction()

#[[
 currently does nothing on tcc as there is no binary instrumentation support
 for tcc

 usage: add_coverage_reports(
  TEST_PROJECT_NAME <name>
  TEST_PROFILE_OUTPUT_DIRECTORY <path>
  TEST_REPORT_OUTPUT_DIRECTORY <path>
  TEST_LINK_INSTRUMENTED_DEPENDENCIES [<target name>...]
 )
]]
function("${FUNCTION_PREFIX}add_coverage_targets")
 message(STATUS "${PROJECT_NAME}: Coverage targets are not supported on tcc!")

 #parse arguments
 cmake_parse_arguments(
  act
  ""
  "TEST_PROJECT_NAME;TEST_PROFILE_OUTPUT_DIRECTORY;TEST_REPORT_OUTPUT_DIRECTORY"
  "TEST_LINK_INSTRUMENTED_DEPENDENCIES"
  ${ARGN}
 )

 #[[validate arguments]]
 #ensure we were provided with a valid test name
 string(LENGTH "${act_TEST_PROJECT_NAME}" act_TEST_PROJECT_NAME_LENGTH)
 if(${act_TEST_PROJECT_NAME_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test project name!")
 endif()
 unset(act_TEST_PROJECT_NAME_LENGTH)

 #ensure we were provided with a valid test profile output directory
 string(LENGTH "${act_TEST_PROFILE_OUTPUT_DIRECTORY}" act_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH)
 if(${act_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test profile output directory name!")
 endif()
 unset(act_TEST_PROFILE_OUTPUT_DIRECTORY_LENGTH)

 #ensure we were provided with a valid test report output directory
 string(LENGTH "${act_TEST_REPORT_OUTPUT_DIRECTORY}" act_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH)
 if(${act_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH} EQUAL 0)
  message(FATAL_ERROR "Missing test report output directory name!")
 endif()
 unset(act_TEST_REPORT_OUTPUT_DIRECTORY_LENGTH)

 #prefix for all target names created here
 set(target_name_prefix "test-${act_TEST_PROJECT_NAME}")

 #[[targets to clean up profile and report data]]
 #[[
  target to clean up only the gcov report output directory

  NOTE: not added to the global `clean-coverage` target as the
  `${target_name_prefix}-clean-profile-reports` target, defined below, nukes
  the entire parent dierctory; this target is needed only for the generation of
  individual gcov reports as it cleans up and stale gcov reports, while keeping
  the rest of the `${act_TEST_REPORT_OUTPUT_DIRECTORY}` in-tact
 ]]
 set(gcov_report_output_dir "${act_TEST_REPORT_OUTPUT_DIRECTORY}/gcov")
 add_custom_target(
  "${target_name_prefix}-clean-gcov-reports"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash -c "echo 'Cleaning \"${act_TEST_PROJECT_NAME}\" test project gcov report output directory...'; rm -r '${gcov_report_output_dir}' &> /dev/null; exit 0"
  VERBATIM
 )

 #target to clean up static HTML reports
 add_custom_target(
  "${target_name_prefix}-clean-profile-reports"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash -c "echo 'Cleaning \"${act_TEST_PROJECT_NAME}\" test project report output directory...'; rm -r '${act_TEST_REPORT_OUTPUT_DIRECTORY}' &> /dev/null; exit 0"
  VERBATIM
 )
 add_dependencies(clean-coverage "${target_name_prefix}-clean-profile-reports")

 #skip adding coverage targets if tests are not enabled for this build
 if(
  NOT DEFINED "${VARIABLE_PREFIX}_TESTS_ENABLED"
  OR NOT ${${VARIABLE_PREFIX}_TESTS_ENABLED}
 )
  return()
 endif()
endfunction()
