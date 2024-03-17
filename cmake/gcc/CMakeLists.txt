include_guard(GLOBAL)

#[[
 this file contains the implementations of all build functions used in the
 top-level `CMakeLists.txt` for gcc
]]

#[[ensure required utilities are installed]]
list(
 APPEND required-binaries
  gcov
  gcovr
)
foreach(bin IN LISTS required-binaries)
 unset(found)
 find_program(
  found
  NAMES "${bin}"
 )
 string(FIND "${found}" "NOTFOUND" found)
 if(NOT found STREQUAL -1)
  message(
   FATAL_ERROR
   "Could not find the '${bin}' tool which is required in order to build this "
   "project!"
  )
 endif()
endforeach()
unset(found)
unset(required-binaries)

#[[
 adds the default build flags to the `${PROJECT_NAME}_COMPILER_FLAGS`
 and `${PROJECT_NAME}_LINKER_FLAGS` lists

 usage: add_default_flags()
]]
function("${FUNCTION_PREFIX}add_default_flags")
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 list(
  APPEND "${compiler_flags_var}"
   -Werror
   -Wall
   -Wextra
   -Wshadow
   -pedantic
 )
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 adds debug or non-debug flags, based on the value of
 `${${debug_flag_var}}`, to the `${PROJECT_NAME}_COMPILER_FLAGS`
 and `${PROJECT_NAME}_LINKER_FLAGS` lists

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
 set(linker_flags_var "${VARIABLE_PREFIX}_LINKER_FLAGS")
 if(${${debug_flag_var}})
  message(STATUS "${PROJECT_NAME}: Debug flags enabled")
  list(
   APPEND "${compiler_flags_var}"
    -D${debug_flag_var}
    -O0
    -g
    -fno-inline
    -fno-inline-functions
    -fkeep-inline-functions
    -fkeep-static-functions
  )
 else()
  message(STATUS "${PROJECT_NAME}: Debug flags disabled")
  list(
   APPEND "${compiler_flags_var}"
    -O2
    -flto
  )
  list(
   APPEND "${linker_flags_var}"
    -flto
  )
 endif()

 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
 set(
  "${linker_flags_var}"
  "${${linker_flags_var}}"
  PARENT_SCOPE
 )
endfunction()

#[[
 adds sanitizer flags, based on the value of `${${sanitizer_flag_var}}`,
 to the `${PROJECT_NAME}_COMPILER_FLAGS` and `${PROJECT_NAME}_LINKER_FLAGS`
 lists

 usage: add_sanitizer_flags(<flag variable>)
]]
function("${FUNCTION_PREFIX}add_sanitizer_flags" sanitizer_flag_var)
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
 set(linker_flags_var "${VARIABLE_PREFIX}_LINKER_FLAGS")
 if(${${sanitizer_flag_var}})
  message(STATUS "${PROJECT_NAME}: Sanitizer flags enabled")
  list(
   APPEND "${compiler_flags_var}"
    -D${sanitizer_flag_var}
    -fsanitize=address
    -fsanitize=undefined
  )
  list(
   APPEND "${linker_flags_var}"
    -lasan
    -lubsan
  )
 else()
  message(STATUS "${PROJECT_NAME}: Sanitizer flags disabled")
 endif()

 #propagate compiler and linker flags to global scope
 set(
  "${compiler_flags_var}"
  "${${compiler_flags_var}}"
  PARENT_SCOPE
 )
 set(
  "${linker_flags_var}"
  "${${linker_flags_var}}"
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

 #add test/non-test flags
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
 adds an instrumented test using the compiler and linker flags specified in the
 `${PROJECT_NAME}_COMPILER_FLAGS` and `${PROJECT_NAME}_LINKER_FLAGS` lists

 usage: add_test(
  TEST_NAME <test name>
  TEST_OUTPUT_DIRECTORY <path>
  TEST_SOURCES <path> [<path>...]
  TEST_INCLUDE_DIRECTORIES [<path>...]
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

 #[[add test instrumentation flags to instrumented targets]]
 list(
  APPEND compiler_coverage_flags
   #coverage flags
   #[[
    NOTE: haven't decided if we should also change the .gcno output directory
    (-fprofile-note=...) to mirror the .gcda profile output data (see
    `finalize_targets()` below).
   ]]
   -fprofile-arcs
   -fprofile-values
   -ftest-coverage
   #NOTE: haven't decided yet if we want coverage-based optimizations
   -fprofile-generate
 )
 list(
  APPEND linker_coverage_flags
   #coverage flags
   -fprofile-arcs
   -fprofile-values
   -lgcov
 )
 if(has_instrumented_dependencies)
  foreach(target IN LISTS at_TEST_LINK_INSTRUMENTED_DEPENDENCIES)
   target_compile_options("${target}" PRIVATE ${compiler_coverage_flags})
   target_link_options("${target}" PRIVATE ${linker_coverage_flags})
  endforeach()
 endif()

 #[[set up test executable]]
 #project compile and link flag variables
 set(compiler_flags_var "${VARIABLE_PREFIX}_COMPILER_FLAGS")
 set(linker_flags_var "${VARIABLE_PREFIX}_LINKER_FLAGS")

 #add test executable
 add_executable("${at_TEST_NAME}" ${at_TEST_SOURCES})
 target_compile_options("${at_TEST_NAME}" PUBLIC ${${compiler_flags_var}})
 target_link_options("${at_TEST_NAME}" PUBLIC ${${linker_flags_var}})

 #add coverage flags to executable so all necessary symbols are added
 target_compile_options("${at_TEST_NAME}" PUBLIC ${compiler_coverage_flags})
 target_link_options("${at_TEST_NAME}" PUBLIC ${linker_coverage_flags})

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

 #set test environemnt variables for profiling output
 set_property(
  TEST "${at_TEST_NAME}"
  PROPERTY ENVIRONMENT "LSAN_OPTIONS=suppressions=\"${CMAKE_CURRENT_FUNCTION_LIST_DIR}/lsan-suppressions.txt\""
 )

 #add custom commands to generate profile and report output directories
 add_custom_command(
  OUTPUT "${at_TEST_PROFILE_OUTPUT_DIRECTORY}"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash ARGS -c "echo 'Creating \"${at_TEST_PROJECT_NAME}\" test project profiling output directory...'; mkdir -p '${at_TEST_PROFILE_OUTPUT_DIRECTORY}'"
  VERBATIM
 )
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
  DEPENDS
   "${at_TEST_PROFILE_OUTPUT_DIRECTORY}"
   "${at_TEST_REPORT_OUTPUT_DIRECTORY}"
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
 add targets to generate and clean coverage reports for instrumented tests, if
 any

 usage: add_coverage_reports(
  TEST_PROJECT_NAME <name>
  TEST_PROFILE_OUTPUT_DIRECTORY <path>
  TEST_REPORT_OUTPUT_DIRECTORY <path>
  TEST_LINK_INSTRUMENTED_DEPENDENCIES [<target name>...]
 )
]]
function("${FUNCTION_PREFIX}add_coverage_targets")
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

 #target to clean up the entire `${act_TEST_PROFILE_OUTPUT_DIRECTORY}` directory
 add_custom_target(
  "${target_name_prefix}-clean-profile-data"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash -c "echo 'Cleaning \"${act_TEST_PROJECT_NAME}\" test project profiling output directory...'; rm -r '${act_TEST_PROFILE_OUTPUT_DIRECTORY}' &> /dev/null; exit 0"
  VERBATIM
 )
 add_dependencies(clean-coverage "${target_name_prefix}-clean-profile-data")

 #target to clean up static HTML reports
 add_custom_target(
  "${target_name_prefix}-clean-profile-reports"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash -c "echo 'Cleaning \"${act_TEST_PROJECT_NAME}\" test project report output directory...'; rm -r '${act_TEST_REPORT_OUTPUT_DIRECTORY}' &> /dev/null; exit 0"
  VERBATIM
 )
 add_dependencies(clean-coverage "${target_name_prefix}-clean-profile-reports")

 #[[
  target to clean up gcda per-test-run instrumentation data

  NOTE: populated below as there is a clean command per object file
 ]]
 add_custom_target(
  "${target_name_prefix}-clean-gcov-profile-data"
  WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
  COMMAND /usr/bin/bash -c "exit 0"
  VERBATIM
 )
 add_dependencies(clean-coverage "${target_name_prefix}-clean-gcov-profile-data")

 #skip adding coverage targets if tests are not enabled for this build
 if(
  NOT DEFINED "${VARIABLE_PREFIX}_TESTS_ENABLED"
  OR NOT ${${VARIABLE_PREFIX}_TESTS_ENABLED}
 )
  return()
 endif()

 #[[TODO: set up commands to generate gcov instrumentation data]]
 foreach(target IN LISTS act_TEST_LINK_INSTRUMENTED_DEPENDENCIES)
  #get list of object outputs for the given instrumented dependency
  set(target_object_dir "${PROJECT_BINARY_DIR}/CMakeFiles/${target}.dir")
  file(TO_CMAKE_PATH "${target_object_dir}" target_object_dir)
  get_target_property(
   sources
   "${target}"
   SOURCES
  )

  foreach(source IN LISTS sources)
   #change path to be relative to `${PROJECT_SOURCE_DIR}`
   file(TO_CMAKE_PATH "${source}" source_path)
   cmake_path(
    RELATIVE_PATH source_path
    BASE_DIRECTORY "${PROJECT_SOURCE_DIR}"
    OUTPUT_VARIABLE relative_object_path
   )
   #change path to be `${PROJECT_BINARY_DIR}/CMakeFiles/${target}.dir/...`
   set(object_path "${target_object_dir}/${relative_object_path}")
   cmake_path(GET relative_object_path STEM source_file_stem)
   cmake_path(GET relative_object_path PARENT_PATH relative_object_parent_path)

   #add custom target to generate gcov report(s) for the current source file
   set(gcov_out_dir "${act_TEST_REPORT_OUTPUT_DIRECTORY}/gcov/${relative_object_parent_path}")
   set(gcov_object_dir "${target_object_dir}/${relative_object_parent_path}")
   set(gcov_object_file "${object_path}.o")
   string(SHA256 object_file_name_hash "${gcov_object_file}")
   set(gcov_target_name "${target_name_prefix}-generate-gcov-${object_file_name_hash}")
   add_custom_target(
    "${gcov_target_name}"
    DEPENDS "${target_name_prefix}-clean-gcov-reports"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    #create the destination directory, if it doesn't exist
    COMMAND /usr/bin/bash -c "mkdir -p '${gcov_out_dir}'"
    #generate gcov report(s)
    COMMAND /usr/bin/bash -c "cd '${gcov_out_dir}' && gcov -a -b -c -f -m -l -o '${gcov_object_dir}' '${gcov_object_file}'"
    VERBATIM
   )
   list(APPEND gcov_outputs "${gcov_target_name}")
   unset(gcov_target_name)
   unset(gcov_object_file)
   unset(gcov_object_dir)
   unset(gcov_out_dir)

   #add custom target to clean up the gcda data
   set(gcda "${object_path}.gcda")
   message(STATUS "gcda: ${gcda}")
   string(SHA256 gcda_clean_target_name "${gcda}")
   set(gcda_clean_target_name "${target_name_prefix}-clean-gcda-${gcda_clean_target_name}")
   add_custom_target(
    "${gcda_clean_target_name}"
    WORKING_DIRECTORY "${PROJECT_BINARY_DIR}"
    COMMAND /usr/bin/bash -c "echo 'Cleaning gcda \"${gcda}\"...'; rm '${gcda}' &> /dev/null; exit 0"
    VERBATIM
   )
   add_dependencies(
    "${target_name_prefix}-clean-gcov-profile-data"
    "${gcda_clean_target_name}"
   )
  endforeach()
  unset(gcda_clean_target_name)
  unset(gcov)
  unset(gcov_out_dir)
  unset(gcda)
  unset(gcno)
  unset(relative_object_parent_path)
  unset(source_file_stem)
  unset(object_path)
  unset(relative_object_path)
  unset(source_path)
 endforeach()

 #add custom target to run all llvm gcov merge commands
 add_custom_target(
  "${target_name_prefix}-gcov-reports"
  DEPENDS ${gcov_outputs}
 )
 add_dependencies(coverage "${target_name_prefix}-gcov-reports")

 #TODO: add custom command and target to generate report with gcovr
 set(report_output_dir "${act_TEST_REPORT_OUTPUT_DIRECTORY}/gcovr/")
 foreach(target IN LISTS act_TEST_LINK_INSTRUMENTED_DEPENDENCIES)
  list(APPEND gcovr_target_directories "${PROJECT_BINARY_DIR}/CMakeFiles/${target}.dir")
 endforeach()
 string(
  APPEND cmd
   "gcovr "
   "-v "
   "--html-details "
   [["]] "${report_output_dir}" [[" ]]
   "--html-self-contained "
   "--html-theme github.dark-blue "
   [["]] "${gcovr_target_directories}" [["]]
 )
 add_custom_command(
  OUTPUT "${report_output_dir}"
  DEPENDS "${target_name_prefix}-gcov-reports"
  WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
  COMMAND /usr/bin/bash ARGS -c "${cmd}"
  VERBATIM
 )
 add_custom_target(
  "${target_name_prefix}-gcovr"
  DEPENDS "${report_output_dir}"
 )
 unset(gcovr_report_files)
 unset(report_output_dir)

 #add gcovr report as a dependency of the coverage target
 add_dependencies(coverage "${target_name_prefix}-gcovr")
endfunction()
