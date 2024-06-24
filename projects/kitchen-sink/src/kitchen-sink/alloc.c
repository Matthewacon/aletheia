#define KS_INTERNAL
 #include <kitchen-sink/internal/alloc.h>
#undef KS_INTERNAL

#include <kitchen-sink/alloc.h>

/*TODO:
 * - impl
 * - static errors and error types
 */

//`alloc_free` implementation
void KS_NAME(alloc_free)(void * ptr) {
 KS_NAME(internal_alloc_free)(ptr);
}

//TODO: `alloc` implementation
void * KS_NAME(alloc)(KS_NAME(error_t) * error, size_t const size) {
 KS_NAME(internal_alloc_result_t) result = KS_NAME(internal_alloc)(size);
 //TODO: return static or dynamic error, based on global preference
 if (result.error) {

 }

 return result.result;
}

//TODO: `alloc_aligned` implementation
void * KS_NAME(alloc_aligned)(
 KS_NAME(error_t) * error,
 size_t const size,
 size_t const alignment
) {
 KS_NAME(internal_alloc_result_t) result = KS_NAME(internal_alloc_aligned)(
  size,
  alignment
 );

 //TODO: return static or dynamic error, based on globl preference
 if (result.error) {

 }

 return result.result;
}

//TODO: `alloc_zero` implementation
void * KS_NAME(alloc_zero)(KS_NAME(error_t) * error, size_t const size) {
 KS_NAME(internal_alloc_result_t) result = KS_NAME(internal_alloc_zero)(
  size,
  alignment
 );

 //TODO: return static or dynamic error, based on globl preference
 if (result.error) {

 }

 return result.result;
}

//TODO: `alloc_aligned_zero` implementation
void * KS_NAME(alloc_aligned_zero)(
 KS_NAME(error_t) * error,
 size_t const size,
 size_t const alignment
) {
 KS_NAME(internal_alloc_result_t) result =
  KS_NAME(internal_alloc_aligned_zero)(size, alignment);

 //TODO: return static or dynamic error, based on globl preference
 if (result.error) {

 }

 return result.result;
}

//TODO: `alloc_realloc` implementation
void * KS_NAME(alloc_alloc_realloc)(
 KS_NAME(error_t) * error,
 void * ptr,
 size_t const new_size
) {
 KS_NAME(internal_alloc_result_t) result =
  KS_NAME(internal_alloc_realloc)(size, alignment);

 //TODO: return static or dynamic error, based on globl preference
 if (result.error) {

 }

 return result.result;
}

//TODO: `alloc_realloc_zero`
void * KS_NAME(alloc_realloc_zero)(
 KS_NAME(error_t) * error,
 void * ptr,
 size_t const new_size
) {
 KS_NAME(internal_alloc_result_t) result =
  KS_NAME(internal_alloc_realloc_zero)(size, alignment);

 //TODO: return static or dynamic error, based on globl preference
 if (result.error) {

 }

 return result.result;
}
