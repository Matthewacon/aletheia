/*this file contains tests for the minimal aletheia test system; do not use
 *the definitions in `<aletheia/test.h>` to create tests here
 */
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <aletheia/test.h>
#include <aletheia/util/string.h>

//utility assert functions
static void assert_no_error_impl(
 char const * error,
 char const * expr,
 int line
) {
 if (!error) {
  return;
 }

 printf(
  "error assertion failed on line %d: `%s`\nerror:\n%s\n",
  line,
  expr,
  error
 );
 exit(-1);
}

#define assert_no_error(expr) \
assert_no_error_impl(expr, #expr, __LINE__)

static void assert_true_impl(bool value, char const * expr, int line) {
 if (value) {
  return;
 }
 printf(
  "expression assertion failed on line %d: `%s`\n",
  line,
  expr
 );
 exit(-1);
}

#define assert_true(expr) \
assert_true_impl((bool)(expr), #expr, __LINE__)

#define assert_false(expr) \
assert_true_impl(!(bool)(expr), "!(" #expr ")", __LINE__)

//TODO: test globals
typedef struct {
 size_t
  //callback invocation count
  invoked_count,
  //size of `invoked_values`
  invoked_size;
 //arguments for each invocation `void *[invoked_count]`
 void ** invoked_values;
 //free callbacks for invoked values
 void (**invoked_value_free)(void *);
} callback_info_t;

static void callback_info_zero(callback_info_t * info) {
 //zero everything
 info->invoked_count = 0;
 info->invoked_size = 0;
 info->invoked_values = NULL;
 info->invoked_value_free = NULL;
}

static void callback_info_free(callback_info_t * info) {
 //save values we need to clean up
 size_t const invoked_count = info->invoked_count;
 void ** invoked_values = info->invoked_values;
 void (**invoked_value_free)(void *) = info->invoked_value_free;

 //zero everything
 callback_info_zero(info);

 //free saved values
 for (size_t i = 0; i < invoked_count; i++) {
  if (invoked_value_free[i]) {
   invoked_value_free[i](invoked_values[i]);
  } else {
   free(invoked_values[i]);
  }
 }
 free((void *)invoked_values);
 free((void *)invoked_value_free);
}

static void callback_info_invoked_values_grow_if_needed(callback_info_t * info) {
 if (info->invoked_count + 1 <= info->invoked_size) {
  return;
 }

 //create default storage, if uninitialized
 if (!info->invoked_values) {
  info->invoked_size = 3;
  info->invoked_values = calloc(info->invoked_size, sizeof(void *));
  info->invoked_value_free = calloc(info->invoked_size, sizeof(void (*)(void *)));
  return;
 }

 //grow value storage if already initialized
 size_t const new_size = 2 * info->invoked_size;
 void * copy = calloc(new_size, sizeof(void *));
 memcpy(
  copy,
  info->invoked_values,
  sizeof(void *) * info->invoked_count
 );
 void * to_free = info->invoked_values;
 info->invoked_values = copy;
 free(to_free);

 //grow value free fptr storage if already initialized
 copy = calloc(new_size, sizeof(void (*)(void *)));
 memcpy(
  copy,
  info->invoked_value_free,
  sizeof(void (*)(void *)) * info->invoked_count
 );
 to_free = info->invoked_value_free;
 info->invoked_value_free = copy;
 free(to_free);

 //update size
 info->invoked_size = new_size;
}

static void callback_info_push_invocation(
 callback_info_t * info,
 void * arg,
 void (*arg_free)(void *)
) {
 callback_info_invoked_values_grow_if_needed(info);
 info->invoked_values[info->invoked_count] = arg;
 info->invoked_value_free[info->invoked_count] = arg_free;
 info->invoked_count++;
}

static struct {
 callback_info_t
  test_callback,
  test_opt_fail_callback,
  test_fail_callback,
  suite_before_each_callback,
  suite_before_each_fail_callback,
  suite_before_each_ctx_callback,
  suite_after_each_callback,
  suite_after_each_fail_callback,
  suite_after_each_ctx_callback,
  suite_before_all_callback,
  suite_before_all_fail_callback,
  suite_before_all_ctx_callback,
  suite_after_all_callback,
  suite_after_all_fail_callback,
  suite_after_all_ctx_callback;
} global_test_data;

static callback_info_t * all_global_test_data[] = {
 &global_test_data.test_callback,
 &global_test_data.test_opt_fail_callback,
 &global_test_data.test_fail_callback,
 &global_test_data.suite_before_each_callback,
 &global_test_data.suite_before_each_fail_callback,
 &global_test_data.suite_before_each_ctx_callback,
 &global_test_data.suite_after_each_callback,
 &global_test_data.suite_after_each_fail_callback,
 &global_test_data.suite_after_each_ctx_callback,
 &global_test_data.suite_before_all_callback,
 &global_test_data.suite_before_all_fail_callback,
 &global_test_data.suite_before_all_ctx_callback,
 &global_test_data.suite_after_all_callback,
 &global_test_data.suite_after_all_fail_callback,
 &global_test_data.suite_after_all_ctx_callback
};

static void zero_test_globals(void) {
 size_t const size = sizeof(all_global_test_data)/sizeof(all_global_test_data[0]);
 for (size_t i = 0; i < size; i++) {
  callback_info_zero(all_global_test_data[i]);
 }
}

static void reset_test_globals(void) {
 size_t const size = sizeof(all_global_test_data)/sizeof(all_global_test_data[0]);
 for (size_t i = 0; i < size; i++) {
  callback_info_free(all_global_test_data[i]);
 }
}

typedef struct {
 char const * name;
 void * ctx;
} test_info_t;

static void test_info_free(void * p) {
 test_info_t * info = p;
 free((void *)info->name);
 free((void *)info);
}

//TODO: pull all info from `test` and push argument for all callbacks
static void global_test_callback(test_t test, void * ctx) {
 //push argument
 (void)ctx;
 callback_info_t * info = &global_test_data.test_callback;
 test_info_t * arg = calloc(1, sizeof(test_info_t));
 assert_no_error(test_get_name(&test, &arg->name));
 arg->ctx = ctx;
 callback_info_push_invocation(info, arg, test_info_free);

 //mark test ok
 test_ok(&test);
}

static void global_test_fail_callback(test_t test, void * ctx) {
 //push argument
 (void)ctx;
 callback_info_t * info = &global_test_data.test_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //mark test opt fail
 assert_no_error(test_push_failure(
  &test,
  "another file",
  829,
  "uh oh"
 ));
}

static void global_test_opt_fail_callback(test_t test, void * ctx) {
 //push argument
 (void)ctx;
 callback_info_t * info = &global_test_data.test_opt_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //mark test opt fail
 assert_no_error(test_push_opt_failure(
  &test,
  "some file",
  543,
  "pebcak"
 ));
}

typedef struct {
 char const * name;
 void * ctx;
} test_setup_info_t;

static void test_setup_info_free(void * p) {
 test_setup_info_t * info = p;
 free((void *)info->name);
 free((void *)info);
}

static void global_before_each_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_each_callback;

 test_t test = test_runner_setup_get_test(&setup);
 assert_true(test != NULL);
 char const * name;
 assert_no_error(test_get_name(&test, &name));
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = name;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

static void global_before_each_fail_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_each_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //fail callback
 assert_no_error(test_runner_setup_fail(&setup, "something went wrong"));
}

static void global_before_each_ctx_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_each_ctx_callback;
 test_t test = test_runner_setup_get_test(&setup);
 assert_true(test != NULL);
 char const * name;
 assert_no_error(test_get_name(&test, &name));
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = name;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);

 //set test ctx
 test_runner_setup_set_ctx(&setup, (void *)info->invoked_count);
}

static void global_after_each_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_after_each_callback;

 test_t test = test_runner_setup_get_test(&setup);
 assert_true(test != NULL);
 char const * name;
 assert_no_error(test_get_name(&test, &name));
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = name;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

static void global_after_each_fail_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_after_each_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //fail callback
 assert_no_error(test_runner_setup_fail(&setup, "uh what"));
}

static void global_after_each_ctx_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_after_each_ctx_callback;
 test_t test = test_runner_setup_get_test(&setup);
 assert_true(test != NULL);
 char const * name;
 assert_no_error(test_get_name(&test, &name));
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = name;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

static void global_before_all_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_all_callback;
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = NULL;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

static void global_before_all_fail_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_all_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //fail callback
 assert_no_error(test_runner_setup_fail(&setup, "i don't like this test"));
}

static void global_before_all_ctx_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_before_all_ctx_callback;
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = NULL;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);

 //set test ctx
 test_runner_setup_set_ctx(&setup, (void *)info->invoked_count);
}

static void global_after_all_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_after_all_callback;
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = NULL;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

static void global_after_all_fail_callback(test_runner_setup_t setup) {
 //push argument
 (void)setup;
 callback_info_t * info = &global_test_data.suite_after_all_fail_callback;
 callback_info_push_invocation(info, NULL, NULL);

 //fail callback
 assert_no_error(test_runner_setup_fail(&setup, "i didn't like that test"));
}

static void global_after_all_ctx_callback(test_runner_setup_t setup) {
 //push argument
 callback_info_t * info = &global_test_data.suite_after_all_ctx_callback;
 test_setup_info_t * arg = calloc(1, sizeof(test_setup_info_t));
 arg->name = NULL;
 arg->ctx = test_runner_setup_get_ctx(&setup);
 callback_info_push_invocation(info, arg, test_setup_info_free);
}

//`test_t` tests
static void test__test_t__creation_deletion(void) {
 //construct test
 reset_test_globals();
 char const * const expected_name = "example test 1";
 enum test_status_t const expected_status = TEST_NOT_RUN;
 size_t const expected_failure_count = 0;

 test_t test;
 assert_no_error(test_new(&test, expected_name, global_test_callback));

 //validate test name
 char const * returned_name = NULL;
 assert_no_error(test_get_name(&test, &returned_name));
 assert_true(strcmp(returned_name, expected_name) == 0);
 assert_true((void *)returned_name != (void *)expected_name);
 free((void *)returned_name);
 returned_name = NULL;

 //validate test status
 enum test_status_t returned_status = 0;
 assert_no_error(test_get_status(&test, &returned_status));
 assert_true(returned_status == expected_status);

 //validate test failures
 size_t returned_failure_count = 0;
 test_failure_t * returned_failures = NULL;
 assert_no_error(test_get_failures(
  &test,
  &returned_failure_count,
  &returned_failures
 ));
 assert_true(returned_failure_count == expected_failure_count);
 test_failures_free(&returned_failure_count, &returned_failures);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test
 test_free(&test);
}

static void test__test_t__get_name(void) {
 //construct test
 reset_test_globals();
 char const * const expected_name = "the power of the sun in the palm of your hand";
 test_t test;

 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));

 //validate name
 char const * returned_name;
 assert_no_error(test_get_name(&test, &returned_name));
 assert_true(strcmp(returned_name, expected_name) == 0);
 free((void *)returned_name);

 //destroy test
 test_free(&test);
}

static void test__test_t__get_status(void) {
 //construct test
 reset_test_globals();
 test_t test;
 assert_no_error(test_new(
  &test,
  "woah",
  global_test_callback
 ));

 //validate status
 enum test_status_t returned_status = 0;
 assert_no_error(test_get_status(&test, &returned_status));
 assert_true(returned_status == TEST_NOT_RUN);

 //destroy test
 test_free(&test);
}

static void test__test_t__get_failures(void) {
 //construct test
 reset_test_globals();
 test_t test;
 assert_no_error(test_new(
  &test,
  "no failures here",
  global_test_callback
 ));

 //validate failures
 size_t failure_count;
 test_failure_t * returned_failures;
 assert_no_error(test_get_failures(&test, &failure_count, &returned_failures));
 assert_true(failure_count == 0);
 test_failures_free(&failure_count, &returned_failures);

 //destroy test
 test_free(&test);
}

static void test__test_t__push_failure(void) {
 //construct test
 reset_test_globals();
 char const * const expected_name = "example test 2";
 enum test_status_t const expected_status = TEST_FAIL;
 size_t const expected_failure_count = 1;

 test_t test;
 assert_no_error(test_new(&test, expected_name, global_test_callback));

 //push failure
 char const * const expected_failure_file_name = "failure.c";
 int const expected_failure_line_number = 123;
 char const * const expected_failure_cause = "cosmic rays";
 assert_no_error(test_push_failure(
  &test,
  expected_failure_file_name,
  expected_failure_line_number,
  expected_failure_cause
 ));

 //validate test state
 enum test_status_t returned_status = 0;
 assert_no_error(test_get_status(&test, &returned_status));
 assert_true(returned_status == expected_status);

 //validate failures
 size_t returned_failure_count = 0;
 test_failure_t * returned_failures = NULL;
 assert_no_error(test_get_failures(
  &test,
  &returned_failure_count,
  &returned_failures
 ));
 assert_true(returned_failure_count == expected_failure_count);
 assert_true(returned_failures != NULL);
 assert_true(strcmp(returned_failures[0].file, expected_failure_file_name) == 0);
 assert_true(returned_failures[0].line == expected_failure_line_number);
 assert_true(strcmp(returned_failures[0].cause, expected_failure_cause) == 0);
 test_failures_free(&returned_failure_count, &returned_failures);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test
 test_free(&test);
}

static void test__test_t__push_opt_failure(void) {
 //construct test
 reset_test_globals();
 char const * const expected_name = "example test 3";
 enum test_status_t const expected_status = TEST_OK_OTHER_FAIL;
 size_t const expected_failure_count = 1;

 test_t test;
 assert_no_error(test_new(&test, expected_name, global_test_callback));

 //push failure
 char const * const expected_failure_file_name = "opt_example.c";
 int const expected_failure_line_number = 456;
 char const * const expected_failure_cause = "too polite";
 assert_no_error(test_push_opt_failure(
  &test,
  expected_failure_file_name,
  expected_failure_line_number,
  expected_failure_cause
 ));

 //validate test state
 enum test_status_t returned_status = 0;
 assert_no_error(test_get_status(&test, &returned_status));
 assert_true(returned_status == expected_status);

 //validate failures
 size_t returned_failure_count = 0;
 test_failure_t * returned_failures = NULL;
 assert_no_error(test_get_failures(
  &test,
  &returned_failure_count,
  &returned_failures
 ));
 assert_true(returned_failure_count == expected_failure_count);
 assert_true(returned_failures != NULL);
 assert_true(strcmp(returned_failures[0].file, expected_failure_file_name) == 0);
 assert_true(returned_failures[0].line == expected_failure_line_number);
 assert_true(strcmp(returned_failures[0].cause, expected_failure_cause) == 0);
 test_failures_free(&returned_failure_count, &returned_failures);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test
 test_free(&test);
}

static void test__test_t__test_ok(void) {
 //construct test
 reset_test_globals();
 char const * const expected_test_name = "example test 4";
 enum test_status_t const expected_status = TEST_OK;
 size_t const expected_failure_count = 0;

 test_t test;
 assert_no_error(test_new(&test, expected_test_name, global_test_callback));

 //push test ok
 test_ok(&test);

 //validate test status
 enum test_status_t returned_status = 0;
 assert_no_error(test_get_status(&test, &returned_status));
 assert_true(returned_status == expected_status);

 //validate failures
 size_t returned_failure_count = 0;
 test_failure_t * returned_failures = NULL;
 assert_no_error(test_get_failures(
  &test,
  &returned_failure_count,
  &returned_failures
 ));
 assert_true(returned_failure_count == expected_failure_count);
 test_failures_free(&returned_failure_count, &returned_failures);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test
 test_free(&test);
}

//TODO: `test_suite_t` tests
static void test__test_suite_t__creation_deletion(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //destory test
 test_suite_free(&test_suite);
}

static void test__test_suite_t__add_test(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct test
 test_t test;
 assert_no_error(test_new(&test, "example test 5", global_test_callback));

 //add test
 assert_no_error(test_suite_add(&test_suite, &test));

 //TODO: get test

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test
 test_free(&test);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__add_tests(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 30; i++) {
  char const * name = string_format("example test %d", i + 5);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //TODO: get tests

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 30; i++) {
  char const * name = string_format("example test %d", i + 35);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(&test_suite, TEST_RUNNER_DEFAULT);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 30);
 assert_true(result == 0);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__validate_before_each_setup(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 char const * const expected_name = "example test 321";
 test_t test;
 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //run test
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = global_before_each_callback,
   .after_each = NULL,
   .before_all = NULL,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_before_each_callback.invoked_count == 1);
 assert_true(global_test_data.test_callback.invoked_count == 1);
 test_setup_info_t * invocation_data = global_test_data.suite_before_each_callback.invoked_values[0];
 assert_true(strcmp(invocation_data->name, expected_name) == 0);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__validate_after_each_setup(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 char const * const expected_name = "munch";
 test_t test;
 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //run test
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = global_after_each_callback,
   .before_all = NULL,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_after_each_callback.invoked_count == 1);
 assert_true(global_test_data.test_callback.invoked_count == 1);
 test_setup_info_t * invocation_data = global_test_data.suite_after_each_callback.invoked_values[0];
 assert_true(strcmp(invocation_data->name, expected_name) == 0);
 assert_true(invocation_data->ctx == NULL);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__validate_before_all_setup(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 char const * const expected_name = "hunch";
 test_t test;
 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //run test
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = global_before_all_callback,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_before_all_callback.invoked_count == 1);
 assert_true(global_test_data.test_callback.invoked_count == 1);
 test_setup_info_t * invocation_data = global_test_data.suite_before_all_callback.invoked_values[0];
 assert_true(invocation_data->name == NULL);
 assert_true(invocation_data->ctx == NULL);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__validate_after_all_setup(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 char const * const expected_name = "brunch";
 test_t test;
 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //run test
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = NULL,
   .after_all = global_after_all_callback
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_after_all_callback.invoked_count == 1);
 assert_true(global_test_data.test_callback.invoked_count == 1);
 test_setup_info_t * invocation_data = global_test_data.suite_after_all_callback.invoked_values[0];
 assert_true(invocation_data->name == NULL);
 assert_true(invocation_data->ctx == NULL);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__validate_test_argument(void) {
 //construct test
 reset_test_globals();
 char const * const expected_name = "apollo";
 test_t test;
 assert_no_error(test_new(
  &test,
  expected_name,
  global_test_callback
 ));

 //construct suite and add test
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //run test
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  TEST_RUNNER_DEFAULT
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 1);
 test_info_t * invocation_data = (test_info_t *)global_test_data.test_callback.invoked_values[0];
 assert_true(strcmp(invocation_data->name, expected_name) == 0);
 assert_true(invocation_data->ctx == NULL);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_constructors(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 10; i++) {
  char const * name = string_format("example test %d", i + 100);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = global_before_each_callback,
   .after_each = global_after_each_callback,
   .before_all = global_before_all_callback,
   .after_all = global_after_all_callback
  }
 );

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 10);
 assert_true(result == 0);
 assert_true(global_test_data.suite_before_all_callback.invoked_count == 1);
 assert_true(global_test_data.suite_after_all_callback.invoked_count == 1);
 assert_true(global_test_data.suite_before_each_callback.invoked_count == 10);
 assert_true(global_test_data.suite_after_each_callback.invoked_count == 10);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_before_all_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 5; i++) {
  char const * name = string_format("example test %d", i + 200);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = global_before_all_fail_callback,
   .after_all = NULL
  }
 );

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);
 assert_true(result > 0);
 assert_true(global_test_data.suite_before_all_fail_callback.invoked_count == 1);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 5);
 for (size_t i = 0; i < test_count; i++) {
  //validate test status
  enum test_status_t status = 0;
  test_get_status(tests + i, &status);
  assert_true(status == TEST_FAIL);

  //validate failures
  test_failure_t * failures;
  size_t failure_count;
  assert_no_error(test_get_failures(tests + i, &failure_count, &failures));
  assert_true(failure_count == 1);
  assert_true(failures[0].fatal);
  assert_true(strstr(failures[0].cause, "i don't like this test") != NULL);
  test_failures_free(&failure_count, &failures);
  test_free(&tests[i]);
 }
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_after_all_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 5; i++) {
  char const * name = string_format("example test %d", i + 300);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = NULL,
   .after_all = global_after_all_fail_callback
  }
 );

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 5);
 assert_true(result > 0);
 assert_true(global_test_data.suite_after_all_fail_callback.invoked_count == 1);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 5);
 for (size_t i = 0; i < test_count; i++) {
  //validate test status
  enum test_status_t status = 0;
  test_get_status(tests + i, &status);
  assert_true(status == TEST_OK_OTHER_FAIL);

  //validate failures
  test_failure_t * failures;
  size_t failure_count;
  assert_no_error(test_get_failures(tests + i, &failure_count, &failures));
  assert_true(failure_count == 1);
  assert_false(failures[0].fatal);
  assert_true(strstr(failures[0].cause, "i didn't like that test") != NULL);
  test_failures_free(&failure_count, &failures);
  test_free(&tests[i]);
 }
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_before_each_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 5; i++) {
  char const * name = string_format("example test %d", i + 400);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = global_before_each_fail_callback,
   .after_each = NULL,
   .before_all = NULL,
   .after_all = NULL
  }
 );

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);
 assert_true(result > 0);
 assert_true(global_test_data.suite_before_each_fail_callback.invoked_count == 5);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 5);
 for (size_t i = 0; i < test_count; i++) {
  //validate test status
  enum test_status_t status = 0;
  test_get_status(tests + i, &status);
  assert_true(status == TEST_FAIL);

  //validate failures
  test_failure_t * failures;
  size_t failure_count;
  assert_no_error(test_get_failures(tests + i, &failure_count, &failures));
  assert_true(failure_count == 1);
  assert_true(failures[0].fatal);
  assert_true(strstr(failures[0].cause, "something went wrong") != NULL);
  test_failures_free(&failure_count, &failures);
  test_free(&tests[i]);
 }
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_after_each_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 for (size_t i = 0; i < 5; i++) {
  char const * name = string_format("example test %d", i + 500);
  test_t test;
  assert_no_error(test_new(&test, name, global_test_callback));
  free((void *)name);

  //add test
  assert_no_error(test_suite_add(&test_suite, &test));

  //destroy test
  test_free(&test);
 }

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = global_after_each_fail_callback,
   .before_all = NULL,
   .after_all = NULL
  }
 );

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 5);
 assert_true(result > 0);
 assert_true(global_test_data.suite_after_each_fail_callback.invoked_count == 5);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 5);
 for (size_t i = 0; i < test_count; i++) {
  //validate test status
  enum test_status_t status = 0;
  test_get_status(tests + i, &status);
  assert_true(status == TEST_OK_OTHER_FAIL);

  //validate failures
  test_failure_t * failures;
  size_t failure_count;
  assert_no_error(test_get_failures(tests + i, &failure_count, &failures));
  assert_true(failure_count == 1);
  assert_false(failures[0].fatal);
  assert_true(strstr(failures[0].cause, "uh what") != NULL);
  test_failures_free(&failure_count, &failures);
  test_free(&tests[i]);
 }
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_test_with_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 test_t test;
 assert_no_error(test_new(
  &test,
  "example test 600",
  global_test_fail_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_fail_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  TEST_RUNNER_DEFAULT
 );

 //validate external state
 assert_true(global_test_data.test_fail_callback.invoked_count == 1);
 assert_true(result > 0);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 1);

 //validate test status
 enum test_status_t status = 0;
 test_get_status(&tests[0], &status);
 assert_true(status == TEST_FAIL);

 //validate failure
 test_failure_t * failures;
 size_t failure_count;
 assert_no_error(test_get_failures(&tests[0], &failure_count, &failures));
 assert_true(failure_count == 1);
 assert_true(failures[0].fatal);
 assert_true(strstr(failures[0].cause, "uh oh") != NULL);
 test_failures_free(&failure_count, &failures);
 test_free(&tests[0]);
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_test_with_opt_failure(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 test_t test;
 assert_no_error(test_new(
  &test,
  "example test 700",
  global_test_opt_fail_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_opt_fail_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  TEST_RUNNER_DEFAULT
 );

 //validate external state
 assert_true(global_test_data.test_opt_fail_callback.invoked_count == 1);
 assert_true(result > 0);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 1);

 //validate test status
 enum test_status_t status = 0;
 test_get_status(&tests[0], &status);
 assert_true(status == TEST_OK_OTHER_FAIL);

 //validate failure
 test_failure_t * failures;
 size_t failure_count;
 assert_no_error(test_get_failures(&tests[0], &failure_count, &failures));
 assert_true(failure_count == 1);
 assert_false(failures[0].fatal);
 assert_true(strstr(failures[0].cause, "pebcak") != NULL);
 test_failures_free(&failure_count, &failures);
 test_free(&tests[0]);
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__run_tests_with_mixed_failures(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 test_t test;
 assert_no_error(test_new(
  &test,
  "example test 800",
  global_test_opt_fail_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);
 assert_no_error(test_new(
  &test,
  "example test 900",
  global_test_fail_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_opt_fail_callback.invoked_count == 0);
 assert_true(global_test_data.test_fail_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  TEST_RUNNER_DEFAULT
 );

 //validate external state
 assert_true(global_test_data.test_opt_fail_callback.invoked_count == 1);
 assert_true(global_test_data.test_fail_callback.invoked_count == 1);
 assert_true(result > 0);

 //validate test failures
 test_t * tests;
 size_t test_count;
 assert_no_error(test_suite_get_tests(&test_suite, &test_count, &tests));
 assert_true(test_count == 2);

 //validate test status
 enum test_status_t status = 0;
 test_get_status(&tests[0], &status);
 assert_true(status == TEST_OK_OTHER_FAIL);
 test_get_status(&tests[1], &status);
 assert_true(status == TEST_FAIL);

 //validate failure
 test_failure_t * failures;
 size_t failure_count;

 //validate failures from first test
 assert_no_error(test_get_failures(&tests[0], &failure_count, &failures));
 assert_true(failure_count == 1);
 assert_false(failures[0].fatal);
 assert_true(strstr(failures[0].cause, "pebcak") != NULL);
 test_failures_free(&failure_count, &failures);

 //validate second test
 assert_no_error(test_get_failures(&tests[1], &failure_count, &failures));
 assert_true(failure_count == 1);
 assert_true(failures[0].fatal);
 assert_true(strstr(failures[0].cause, "uh oh") != NULL);
 test_failures_free(&failure_count, &failures);

 //clean up retrieved tests from suite
 test_free(&tests[0]);
 test_free(&tests[1]);
 free((void *)tests);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__push_test_ctx_before_each(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 test_t test;
 assert_no_error(test_new(
  &test,
  "example test 900",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);
 assert_no_error(test_new(
  &test,
  "example test 901",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = global_before_each_ctx_callback,
   .after_each = NULL,
   .before_all = NULL,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_before_each_ctx_callback.invoked_count == 2);
 assert_true(global_test_data.test_callback.invoked_count == 2);
 test_info_t ** invocation_data = (test_info_t **)global_test_data.test_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x1);
 assert_true(invocation_data[1]->ctx == (void *)0x2);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__pop_test_ctx_after_each(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add tests
 test_t test;
 assert_no_error(test_new(
  &test,
  "aaa",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);
 assert_no_error(test_new(
  &test,
  "bbb",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = global_before_each_ctx_callback,
   .after_each = global_after_each_ctx_callback,
   .before_all = NULL,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.suite_before_each_ctx_callback.invoked_count == 2);
 assert_true(global_test_data.suite_after_each_ctx_callback.invoked_count == 2);
 assert_true(global_test_data.test_callback.invoked_count == 2);
 test_setup_info_t ** invocation_data = (test_setup_info_t **)global_test_data.suite_before_each_ctx_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x0);
 assert_true(invocation_data[1]->ctx == (void *)0x1);
 invocation_data = (test_setup_info_t **)global_test_data.suite_after_each_ctx_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x1);
 assert_true(invocation_data[1]->ctx == (void *)0x2);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__push_test_ctx_before_all(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 test_t test;
 assert_no_error(test_new(
  &test,
  "example test 1000",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);
 assert_no_error(test_new(
  &test,
  "example test 1001",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = global_before_all_ctx_callback,
   .after_all = NULL
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 2);
 assert_true(global_test_data.suite_before_all_ctx_callback.invoked_count == 1);
 test_info_t ** invocation_data = (test_info_t **)global_test_data.test_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x1);

 //destroy test suite
 test_suite_free(&test_suite);
}

static void test__test_suite_t__pop_test_ctx_after_all(void) {
 //construct test suite
 reset_test_globals();
 test_suite_t test_suite;
 assert_no_error(test_suite_new(&test_suite));

 //construct and add test
 test_t test;
 assert_no_error(test_new(
  &test,
  "hellorld",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);
 assert_no_error(test_new(
  &test,
  "abcd",
  global_test_callback
 ));
 assert_no_error(test_suite_add(&test_suite, &test));
 test_free(&test);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 0);

 //run test suite
 size_t const result = test_suite_run_and_emit(
  &test_suite,
  (test_runner_config_t) {
   .before_each = NULL,
   .after_each = NULL,
   .before_all = global_before_all_ctx_callback,
   .after_all = global_after_all_ctx_callback
  }
 );
 assert_true(result == 0);

 //validate external state
 assert_true(global_test_data.test_callback.invoked_count == 2);
 assert_true(global_test_data.suite_before_all_ctx_callback.invoked_count == 1);
 assert_true(global_test_data.suite_after_all_ctx_callback.invoked_count == 1);
 test_setup_info_t ** invocation_data = (test_setup_info_t **)global_test_data.suite_before_all_ctx_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x0);
 invocation_data = (test_setup_info_t **)global_test_data.suite_after_all_ctx_callback.invoked_values;
 assert_true(invocation_data[0]->ctx == (void *)0x1);

 //destroy test suite
 test_suite_free(&test_suite);
}

//TODO: utility function tests

int main(void) {
 //initialize test globals
 zero_test_globals();

 //`test_t` tests
 test__test_t__creation_deletion();
 test__test_t__get_name();
 test__test_t__get_status();
 test__test_t__get_failures();
 test__test_t__push_failure();
 test__test_t__push_opt_failure();
 test__test_t__test_ok();

 //`test_suite_t` tests
 test__test_suite_t__creation_deletion();
 test__test_suite_t__add_test();
 test__test_suite_t__add_tests();
 test__test_suite_t__run_tests();

 //`test_suite_t`/`test_runner_setup_t` tests
 test__test_suite_t__validate_before_each_setup();
 test__test_suite_t__validate_after_each_setup();
 test__test_suite_t__validate_before_all_setup();
 test__test_suite_t__validate_after_all_setup();
 test__test_suite_t__validate_test_argument();

 test__test_suite_t__push_test_ctx_before_each();
 test__test_suite_t__pop_test_ctx_after_each();
 test__test_suite_t__push_test_ctx_before_all();
 test__test_suite_t__pop_test_ctx_after_all();

 test__test_suite_t__run_tests_with_constructors();
 test__test_suite_t__run_tests_with_before_all_failure();
 test__test_suite_t__run_tests_with_after_all_failure();
 test__test_suite_t__run_tests_with_before_each_failure();
 test__test_suite_t__run_tests_with_after_each_failure();
 test__test_suite_t__run_test_with_failure();
 test__test_suite_t__run_test_with_opt_failure();
 test__test_suite_t__run_tests_with_mixed_failures();

 //TODO: test expr tests

 //clean up remaining globals, if any
 reset_test_globals();

 return 0;
}
