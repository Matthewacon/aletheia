#pragma once

/*TODO:
 * - symbol name switch macro
 * - use debug compiler define
 * - add utilities for discovering tests
 * - benchmark support
 * - test report formats (JSON, HTML)
 * - benchmark report formats (JSON, HTML)
 */

#include <stddef.h>
#include <stdbool.h>
#include <stdint.h>

//test status enum
enum test_status_t {
 TEST_NOT_RUN = 1,
 TEST_OK,
 TEST_FAIL,
 TEST_OK_OTHER_FAIL
};

//descriptor for test failures
typedef struct {
 bool fatal;
 char const * file;
 int line;
 char const * cause;
} test_failure_t;
void test_failure_free(test_failure_t * failure);
void test_failures_free(size_t * count, test_failure_t ** failures);
char const * test_failure_copy(test_failure_t * failure, test_failure_t * dst);

//opaque pointer for test descriptor
typedef uint8_t * test_t;
//opaque pointer for test runner descriptor
typedef uint8_t * test_runner_setup_t;
//opaque pointer for test suite descriptor
typedef uint8_t * test_suite_t;

//`test_runner_setup_t` functions
test_suite_t test_runner_setup_get_test_suite(test_runner_setup_t * setup);
test_t test_runner_setup_get_test(test_runner_setup_t * setup);
void test_runner_setup_set_ctx(test_runner_setup_t * setup, void * ctx);
void * test_runner_setup_get_ctx(test_runner_setup_t * setup);
char const * test_runner_setup_fail(test_runner_setup_t * setup, char const * error);

//options type for test suites
typedef struct {
 //function run before each test
 void (*before_each)(test_runner_setup_t setup);
 //function run after each test
 void (*after_each)(test_runner_setup_t setup);
 //function run before all tests
 void (*before_all)(test_runner_setup_t setup);
 //function run after all tests
 void (*after_all)(test_runner_setup_t setup);
} test_runner_config_t;

//conveinence macro
#define TEST_RUNNER_DEFAULT (test_runner_config_t) {\
 .before_each = NULL,\
 .after_each = NULL,\
 .before_all = NULL,\
 .after_all = NULL\
}

//`test_t` functions
//prototype for test callback
typedef void test_callback_t(test_t test, void * ctx);
char const * test_new(
 test_t * dst,
 char const * name,
 test_callback_t * callback
);
void test_free(test_t * test);
char const * test_copy(test_t * test, test_t * dst);
char const * test_push_failure(
 test_t * test,
 char const * file,
 int line,
 char const * cause
);
char const * test_push_opt_failure(
 test_t * test,
 char const * file,
 int line,
 char const * cause
);
char const * test_ok(test_t * test);
char const * test_get_name(test_t * test, char const ** dst);
char const * test_get_status(test_t * test, enum test_status_t * dst);
char const * test_get_failures(
 test_t * test,
 size_t * count,
 test_failure_t ** dst
);

//`test_suite_t` functions
char const * test_suite_new(test_suite_t * dst);
void test_suite_free(test_suite_t * suite);
char const * test_suite_add(test_suite_t * suite, test_t * test);
char const * test_suite_get_tests(
 test_suite_t * suite,
 size_t * count,
 test_t ** dst
);
size_t test_suite_run_and_emit(
 test_suite_t * suite,
 test_runner_config_t runner_config
);

//utility function for fatal internal errors
void handle_internal_failure(char const * error, char const * func);

#define TEST_SUITE() \
static void add_tests(test_suite_t test_suite);\
int main(void) {\
 test_suite_t test_suite;\
 handle_internal_failure(test_suite_new(&test_suite), __func__);\
 add_tests(test_suite);\
 size_t const result = test_suite_run_and_emit(&test_suite, TEST_RUNNER_DEFAULT);\
 test_suite_free(&test_suite);\
 return (int)result;\
}\
\
static void add_tests(test_suite_t test_suite)

#define TEST(name) {\
 test_t test;\
 handle_internal_failure(test_new(&test, #name, name), __func__);\
 handle_internal_failure(test_suite_add(&test_suite, &test), __func__);\
 test_free(&test);\
}

//test utility functions and macros
typedef struct {
 test_t * test;
 char const * file;
 int line;
 char const * identifier_name;
} test_stmt_t;

bool test_stmt_bool_eq__impl(
 test_stmt_t details,
 bool assertion,
 bool expected,
 bool value
);

//shim to `test_stmt_bool_eq__impl`
#define test_stmt_bool_eq(assertion, expected, value) {\
 bool const failed = test_stmt_bool_eq__impl(\
  (test_stmt_t) {\
   .test = &test,\
   .file = __FILE__,\
   .line = __LINE__,\
   .identifier_name = #value\
  },\
  assertion,\
  expected,\
  value\
 );\
 if (assertion && failed) {\
  return;\
 }\
}
#define test_expect_true(value) test_stmt_bool_eq(false, true, value)
#define test_assert_true(value) test_stmt_bool_eq(true, true, value)
#define test_expect_false(value) test_stmt_bool_eq(false, false, value)
#define test_assert_false(value) test_stmt_bool_eq(true, false, value)
