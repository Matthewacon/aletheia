#include <kitchen-sink/error.h>

KS_ERROR_STATIC_DEFN(
 error_scf_too_high,
 KS_NAME_STR(some_critical_function) "(): something > 100!"
);

void some_critical_function(KS_NAME(error_t) * error, size_t something) {
 if (something > 100) {
  KS_NAME(error_push)(error, KS_NAME_STR(some_critical_function) "(): something > 100!");
  return;
 }
}

int main(void) {

 return 0;
}
