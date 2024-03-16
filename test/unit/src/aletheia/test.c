/*this file contains an example test suite built using the aletheia test suite
 *primitives; definitions in `<aletheia/test.h>` should be used here
 */

#include <stdio.h>
#include <aletheia/test.h>

static void test__example(test_t test, void * ctx) {
 (void)test;
 (void)ctx;
 for (size_t i = 0; i < 100; i++) {
  printf("%zu\n", i);
  test_expect_true(true);
 }
 test_ok(&test);
}

TEST_SUITE() {
 //for (size_t i = 0; i < 100; i++) {
  TEST(test__example);
 //}
}
