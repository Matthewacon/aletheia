#include <aletheia/test.h>
#include <aletheia/util/string.h>

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

//TODO: switch all grow functions to realloc

//`test_failure_free` implementation
void test_failure_free(test_failure_t * failure) {
 test_failure_t to_free = *failure;

 //zero failure
 failure->file = NULL;
 failure->cause = NULL;

 //free cause
 free((void *)to_free.cause);
}

// `test_failures_free` implementation
void test_failures_free(size_t * count, test_failure_t ** failures) {
 //zero destination
 test_failure_t * to_free = *failures;
 size_t const to_free_count = *count;
 *count = 0;
 *failures = NULL;

 //free failures and failure buffer
 for (size_t i = 0; i < to_free_count; i++) {
  test_failure_free(to_free + i);
 }
 free((void *)to_free);
}

char const * test_failure_copy(test_failure_t * failure, test_failure_t * dst) {
 //zero destination
 dst->file = NULL;
 dst->cause = NULL;

 //copy contents
 *dst = *failure;
 //TODO: handle format failures
 dst->cause = string_format("%s", failure->cause);

 return NULL;
}

//`test_t` implementation
typedef struct {
 //test name
 char const * name;
 //test callback
 test_callback_t * callback;
 //test status
 enum test_status_t status;
 //test failure information
 size_t
  failure_count,
  failure_size;
 test_failure_t * failures;
} test_impl_t;

//utility function
static test_impl_t * test_get_impl(test_t * result) {
 return (test_impl_t *)*result;
}

//`test_new` implementation
char const * test_new(
 test_t * dst,
 char const * name,
 test_callback_t * callback
) {
 size_t const default_failure_size = 2;
 *dst = NULL;

 test_impl_t * result = calloc(1, sizeof(test_impl_t));
 if (!result) {
  return "Failed to allocate space for test!";
 }

 //TODO: handle `string_format` failure
 //copy name
 result->name = string_format("%s", name);
 result->callback = callback;
 result->status = TEST_NOT_RUN;
 result->failure_count = 0;
 result->failure_size = default_failure_size;
 result->failures = calloc(default_failure_size, sizeof(test_failure_t));

 //set test in destination
 *dst = (test_t)result;

 return NULL;
}

//utility function for `test_free`
static void test_free_impl(test_impl_t * test_impl) {
 if (!test_impl) {
  return;
 }

 //zero destination
 char const * name = test_impl->name;
 size_t failure_count = test_impl->failure_count;
 test_failure_t * failures = test_impl->failures;
 test_impl->name = NULL;
 test_impl->callback = NULL;
 test_impl->status = 0;
 test_impl->failure_count = 0;
 test_impl->failure_size = 0;
 test_impl->failures = NULL;

 //free name
 free((void *)name);

 //free failures
 test_failures_free(&failure_count, &failures);
}

//`test_free` implementation
void test_free(test_t * test) {
 if (!test || !*test) {
  return;
 }

 //zero destination
 test_impl_t * test_impl = test_get_impl(test);
 *test = NULL;

 //free test contents
 test_free_impl(test_impl);

 //free test
 free((void *)test_impl);
}

//utility for `test_copy`
static char const * test_copy_impl(test_impl_t * test_impl, test_impl_t * dst) {
 //zero destination
 dst->name = NULL;
 dst->callback = NULL;
 dst->status = 0;
 dst->failure_count = 0;
 dst->failure_size = 0;
 dst->failures = NULL;

 //copy all contents
 //TODO: handle string format failure
 dst->name = string_format("%s", test_impl->name);
 dst->callback = test_impl->callback;
 dst->status = test_impl->status;

 //TODO: handle calloc failure
 //copy failures
 char const * error = NULL;
 dst->failures = calloc(
  test_impl->failure_size,
  sizeof(test_failure_t)
 );
 dst->failure_size = test_impl->failure_size;
 for (; dst->failure_count < test_impl->failure_count; dst->failure_count++) {
  error = test_failure_copy(
   test_impl->failures + dst->failure_count,
   dst->failures + dst->failure_count
  );
  if (error) {
   dst->failure_count--;
   break;
  }
 }

 //if error encountered, clean up copy
 if (error) {
  test_free_impl(dst);
  return error;
 }

 return NULL;
}

//`test_copy` implementation
char const * test_copy(test_t * test, test_t * dst) {
 //zero destination
 *dst = NULL;

 test_impl_t
  * test_impl = test_get_impl(test),
  //TODO: handle malloc failure
  * test_copy = calloc(1, sizeof(test_impl_t));

 //copy test
 char const * error = test_copy_impl(test_impl, test_copy);
 if (error) {
  free((void *)test_copy);
  return error;
 }

 //set copy in destination
 *dst = (test_t)test_copy;

 return NULL;
}

//utility function for `test_push_failure` and `test_push_opt_failure`
static void test_grow_failures_if_needed(test_impl_t * test_impl) {
 //if no grow needed, do nothing
 if (test_impl->failure_count + 1 <= test_impl->failure_size) {
  return;
 }

 //TODO: handle `malloc` failure
 //grow failures
 size_t const new_failure_size = test_impl->failure_size * 2;
 test_failure_t * new_failures = calloc(new_failure_size, sizeof(test_failure_t));
 memcpy(
  new_failures,
  test_impl->failures,
  sizeof(test_failure_t) * test_impl->failure_count
 );
 void * to_free = test_impl->failures;
 test_impl->failures = new_failures;
 test_impl->failure_size = new_failure_size;
 free(to_free);
}

//`test_push_failure` implementation
char const * test_push_failure(
 test_t * test,
 char const * file,
 int line,
 char const * cause
) {
 test_impl_t * test_impl = test_get_impl(test);
 test_grow_failures_if_needed(test_impl);

 //TODO: handle `string_format` failure
 //push failure
 test_failure_t failure = {
  .fatal = true,
  .file = file,
  .line = line,
  .cause = string_format("%s", cause)
 };
 memcpy(
  test_impl->failures + test_impl->failure_count,
  &failure,
  sizeof(test_failure_t)
 );
 test_impl->failure_count++;

 //update test status
 test_impl->status = TEST_FAIL;

 return NULL;
}

//`test_push_opt_failure` implementation
char const * test_push_opt_failure(
 test_t * test,
 char const * file,
 int line,
 char const * cause
) {
 test_impl_t * test_impl = test_get_impl(test);
 test_grow_failures_if_needed(test_impl);

 //TODO: handle `string_format` failure
 //push failure
 test_failure_t failure = {
  .fatal = false,
  .file = file,
  .line = line,
  .cause = string_format("%s", cause)
 };
 memcpy(
  test_impl->failures + test_impl->failure_count,
  &failure,
  sizeof(test_failure_t)
 );
 test_impl->failure_count++;

 //only update test status if we did not encounter a fail failure already
 if (test_impl->status != TEST_FAIL) {
  test_impl->status = TEST_OK_OTHER_FAIL;
 }

 return NULL;
}

//`test_ok` implementation
char const * test_ok(test_t * test) {
 test_impl_t * test_impl = test_get_impl(test);
 //only update test status to `TEST_OK` if no failures have occurred
 if (test_impl->status == TEST_NOT_RUN) {
  test_impl->status = TEST_OK;
 }

 return NULL;
}

//`test_get_name` implementation
char const * test_get_name(test_t * test, char const ** dst) {
 *dst = NULL;

 //TODO: handle `string_format` failures
 test_impl_t * test_impl = test_get_impl(test);
 *dst = string_format("%s", test_impl->name);

 return NULL;
}

//`test_get_status` implementation
char const * test_get_status(test_t * test, enum test_status_t * dst) {
 *dst = 0;

 test_impl_t * test_impl = test_get_impl(test);
 *dst = test_impl->status;
 return NULL;
}

//`test_get_failures` implementation
char const * test_get_failures(test_t * test, size_t * count, test_failure_t ** dst) {
 char const * error = NULL;

 //zero destination
 *count = 0;
 *dst = NULL;

 test_impl_t * test_impl = test_get_impl(test);

 //if there are no failures, do nothing
 if (!test_impl->failure_count) {
  return NULL;
 }

 //copy failures
 test_failure_t * copy = calloc(test_impl->failure_count, sizeof(test_failure_t));
 size_t copied = 0;
 for (; copied < test_impl->failure_count; copied++) {
  error = test_failure_copy(test_impl->failures + copied, copy + copied);
  if (error) {
   break;
  }
 }
 //if copying failures failed, free all failures copied thus far
 if (error) {
  test_failures_free(&copied, &copy);
  return error;
 }

 //set results in destination
 *dst = copy;
 *count = test_impl->failure_count;

 return NULL;
}

//`test_runner_setup_t` implementation
typedef struct {
 test_suite_t suite;
 test_t test;
 void * ctx;
 char const * error;
} test_runner_setup_impl_t;

//utility functions
static void test_runner_setup_free(test_runner_setup_impl_t * runner_impl) {
 //zero contents
 void * to_free = (void *)runner_impl->error;
 runner_impl->error = NULL;

 //free error string
 free(to_free);
}

static test_runner_setup_impl_t * test_runner_setup_get_impl(
 test_runner_setup_t * setup
) {
 return (test_runner_setup_impl_t *)*setup;
}

//`test_runner_setup_get_test_suite` implementation
test_suite_t test_runner_setup_get_test_suite(test_runner_setup_t * setup) {
 return test_runner_setup_get_impl(setup)->suite;
}

//`test_runner_setup_get_test` implementation
test_t test_runner_setup_get_test(test_runner_setup_t * setup) {
 return test_runner_setup_get_impl(setup)->test;
}

//`test_runner_setup_set_ctx` implementation
void test_runner_setup_set_ctx(test_runner_setup_t * setup, void * ctx) {
 test_runner_setup_get_impl(setup)->ctx = ctx;
}

//`test_runner_setup_get_ctx` implementation
void * test_runner_setup_get_ctx(test_runner_setup_t * setup) {
 return test_runner_setup_get_impl(setup)->ctx;
}

//`test_runner_setup_fail` implementation
char const * test_runner_setup_fail(test_runner_setup_t * setup, char const * error) {
 test_runner_setup_impl_t * setup_impl = test_runner_setup_get_impl(setup);

 //free previous error
 free((void *)setup_impl->error);
 setup_impl->error = NULL;

 //TODO: handle `string_format` failure
 //copy provided error string
 setup_impl->error = string_format("%s", error);

 return NULL;
}

//`test_suite_t` implementation
typedef struct {
 size_t
  test_count,
  test_size;
 test_impl_t * tests;
} test_suite_impl_t;

//utility function
static test_suite_impl_t * test_suite_get_impl(test_suite_t * suite) {
 return (test_suite_impl_t *)*suite;
}

//`test_suite_new` implementation
char const * test_suite_new(test_suite_t * dst) {
 size_t const default_test_size = 10;

 //zero destination
 *dst = NULL;

 test_suite_impl_t * suite_impl = calloc(1, sizeof(test_suite_impl_t));
 if (!suite_impl) {
  return "Failed to allocate space for test suite";
 }
 suite_impl->test_count = 0;
 suite_impl->test_size = default_test_size;
 suite_impl->tests = calloc(default_test_size, sizeof(test_impl_t));
 *dst = (test_suite_t)suite_impl;

 return NULL;
}

//`test_suite_free` implementation
void test_suite_free(test_suite_t * suite) {
 test_suite_impl_t * suite_impl = test_suite_get_impl(suite);

 //zero destination
 *suite = NULL;

 //free tests
 for (size_t i = 0; i < suite_impl->test_count; i++) {
  test_impl_t * to_free = &suite_impl->tests[i];
  test_free_impl(to_free);
 }
 suite_impl->test_count = 0;
 suite_impl->test_size = 0;

 //free test buffer
 void * to_free = suite_impl->tests;
 suite_impl->tests = NULL;
 free((void *)to_free);

 //free suite
 free((void *)suite_impl);
}

//`test_suite_add` implementation
char const * test_suite_add(test_suite_t * suite, test_t * test) {
 test_suite_impl_t * suite_impl = test_suite_get_impl(suite);
 test_impl_t * test_impl = test_get_impl(test);

 //grow test list if needed
 if (suite_impl->test_count + 1 > suite_impl->test_size) {
  size_t const new_size = suite_impl->test_size * 2;
  test_impl_t * copy = calloc(new_size, sizeof(test_impl_t));
  memcpy(copy, suite_impl->tests, sizeof(test_impl_t) * suite_impl->test_count);
  free((void *)suite_impl->tests);
  suite_impl->tests = copy;
  suite_impl->test_size = new_size;
 }

 //add test
 char const * error = test_copy_impl(
  test_impl,
  suite_impl->tests + suite_impl->test_count
 );
 if (error) {
  return error;
 }
 suite_impl->test_count += 1;

 return NULL;
}

//`test_suite_get_tests` implementation
char const * test_suite_get_tests(
 test_suite_t * suite,
 size_t * count,
 test_t ** dst
) {
 //zero destination
 *dst = NULL;
 *count = 0;

 char const * error = NULL;
 test_suite_impl_t * suite_impl = test_suite_get_impl(suite);

 //TODO: handle malloc failure
 //allocate result buffer
 test_t * result = calloc(suite_impl->test_count, sizeof(test_t));

 //copy all tests
 size_t i = 0;
 for (; i < suite_impl->test_count; i++) {
  test_impl_t * test_impl = &suite_impl->tests[i];
  error = test_copy((test_t *)&test_impl, result + i);
  if (error) {
   break;
  }
 }

 //clean up if we encountered an error
 if (error) {
  for (size_t j = 0; j < i; j++) {
   test_free(result + j);
  }
  free((void *)result);
  return error;
 }

 //set results in destinations
 *dst = result;
 *count = suite_impl->test_count;

 return NULL;
}

//utility functions for `test_suite_run_and_emit`
//TODO: change error handling
static bool test_suite_run_before_all(
 test_runner_config_t * runner_config,
 test_runner_setup_impl_t * runner_impl
) {
 test_suite_impl_t * suite_impl = test_suite_get_impl(&runner_impl->suite);

 //if no callback supplied, do nothing
 if (!runner_config->before_all) {
  return true;
 }

 //run callback
 runner_config->before_all((test_runner_setup_t)runner_impl);

 //if callback succeeded, do nothing
 if (!runner_impl->error) {
  return true;
 }

 //TODO: handle `string_format` failure
 //mark all tests as failed since the suite initializer failed
 char const * cause = string_format(
  "Provided 'before_all()' callback failed with error: %s",
  runner_impl->error
 );
 for (size_t i = 0; i < suite_impl->test_count; i++) {
  test_impl_t * test = &suite_impl->tests[i];
  test_push_failure(
   (test_t *)&test,
   NULL,
   0,
   cause
  );
 }
 free((void *)cause);

 //reset runner impl error state
 free((void *)runner_impl->error);
 runner_impl->error = NULL;

 return false;
}

//TODO: change error handling
static bool test_suite_run_after_all(
 test_runner_config_t * runner_config,
 test_runner_setup_impl_t * runner_impl
) {
 test_suite_impl_t * suite_impl = test_suite_get_impl(&runner_impl->suite);

 //if no callback supplied, do nothing
 if (!runner_config->after_all) {
  return true;
 }

 //run callback
 runner_config->after_all((test_runner_setup_t)runner_impl);

 //if callback succeeded, do nothing
 if (!runner_impl->error) {
  return true;
 }

 //TODO: handle `string_format` failure
 //mark all tests as optionally failed since the suite destructor failed
 char const * cause = string_format(
  "Provided 'after_all()' callback failed with error: %s",
  runner_impl->error
 );
 for (size_t i = 0; i < suite_impl->test_count; i++) {
  test_impl_t * test = &suite_impl->tests[i];
  test_push_opt_failure(
   (test_t *)&test,
   NULL,
   0,
   cause
  );
 }
 free((void *)cause);

 //reset runner impl error state
 free((void *)runner_impl->error);
 runner_impl->error = NULL;

 return false;
}

//TODO: change error handling
static bool test_suite_run_before_each(
 test_runner_config_t * runner_config,
 test_runner_setup_impl_t * runner_impl
) {
 //if no callback supplied, do nothing
 if (!runner_config->before_each) {
  return true;
 }

 //run callback
 runner_config->before_each((test_runner_setup_t)runner_impl);

 //if callback succeeded, do nothing
 if (!runner_impl->error) {
  return true;
 }

 //TODO: handle `string_format` failure
 //mark test as failed
 char const * cause = string_format(
  "Provided 'before_each()' callback failed with error: %s",
  runner_impl->error
 );
 test_push_failure(
  &runner_impl->test,
  NULL,
  0,
  cause
 );
 free((void *)cause);

 //reset runner impl error state
 free((void *)runner_impl->error);
 runner_impl->error = NULL;

 return false;
}

//TODO: change error handling
static bool test_suite_run_after_each(
 test_runner_config_t * runner_config,
 test_runner_setup_impl_t * runner_impl
) {
 //if no callback supplied, do nothing
 if (!runner_config->after_each) {
  return true;
 }

 //run callback
 runner_config->after_each((test_runner_setup_t)runner_impl);

 //if callback succeeded, do nothing
 if (!runner_impl->error) {
  return true;
 }

 //TODO: handle `string_format` failure
 //mark test as optionally failed
 char const * cause = string_format(
  "Provided 'after_each()' callback failed with error: %s",
  runner_impl->error
 );
 test_push_opt_failure(
  &runner_impl->test,
  NULL,
  0,
  cause
 );
 free((void *)cause);

 //reset runner impl error state
 free((void *)runner_impl->error);
 runner_impl->error = NULL;

 return false;
}

//TODO: do the emission part...
//TODO: change error handling
//`test_suite_run_and_emit` implementation
size_t test_suite_run_and_emit(
 test_suite_t * suite,
 test_runner_config_t runner_config
) {
 size_t failures_encountered = 0;
 test_suite_impl_t * suite_impl = test_suite_get_impl(suite);

 //runner ctx for tests
 test_runner_setup_impl_t runner_impl = {
  .suite = (test_suite_t)suite_impl,
  .test = NULL,
  .ctx = NULL,
  .error = NULL
 };

 //run suite initializer; if suite initializer fails, exit immediately
 if (!test_suite_run_before_all(&runner_config, &runner_impl)) {
  test_runner_setup_free(&runner_impl);
  return 1;
 }

 //run all tests
 for (size_t i = 0; i < suite_impl->test_count; i++) {
  test_impl_t * test = &suite_impl->tests[i];
  //update `runner_impl` to point to current test
  runner_impl.test = (test_t)test;

  //run test initializer; if test initializer fails, make note and skip
  if (!test_suite_run_before_each(&runner_config, &runner_impl)) {
   failures_encountered++;
   continue;
  }

  //run test
  test->callback((test_t)test, runner_impl.ctx);

  //TODO: if test did not encounter any failures, call `test_ok`

  //if test failed, make note
  if (test->status != TEST_OK) {
   failures_encountered++;
  }

  //run test destructor; if test destructor fails, make note
  if (!test_suite_run_after_each(&runner_config, &runner_impl)) {
   failures_encountered++;
  }
 }

 //clear last test in `runner_impl`
 runner_impl.test = NULL;

 //run suite destructor; if suite destructor fails, make note
 if (!test_suite_run_after_all(&runner_config, &runner_impl)) {
  failures_encountered++;
 }

 test_runner_setup_free(&runner_impl);
 return failures_encountered;
}

//TODO: test utility function implementations

//utility function for test assert/expect functions
void handle_internal_failure(char const * error, char const * func) {
 if (!error) {
  return;
 }
 printf("internal test suite failure in '%s()': %s\n", func, error);
 exit(-1);
}

//utility typedef for test assert/expect functions
typedef char const * test_fail_func_t(
 test_t * test,
 char const * file,
 int line,
 char const * cause
);

//`test_stmt_bool_eq__impl` implementation
bool test_stmt_bool_eq__impl(
 test_stmt_t details,
 bool assertion,
 bool expected,
 bool value
) {
 if (value == expected) {
  return false;
 }
 char const * cause = string_format(
  "Expected value of '%s' (%s) to be %s!",
  details.identifier_name,
  value ? "true" : "false",
  expected ? "true" : "false"
 );
 test_fail_func_t * fail = assertion ? test_push_failure : test_push_opt_failure;
 char const * error = fail(
  details.test,
  details.file,
  details.line,
  cause
 );
 free((void *)cause);
 handle_internal_failure(error, __func__);
 return true;
}
