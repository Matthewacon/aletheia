#pragma once

//disable header if libc is not the active backend
#if KS_BACKEND_LIB == KS_BACKEND_LIB_LIBC
 #include <kitchen-sink/macros.h>
 #include <kitchen-sink/backends/backends.h>

 //discourage use outside of internal implementations
 #ifndef KS_INTERNAL
  #error \
   "Do not include this header directly, instead include <kitchen-sink/alloc.h>"
 #endif

 //TODO: DOCS

 typedef struct {
  errno_t code;
  char const * error;
  void * result;
 } KS_NAME(libc_alloc_result_t);

 /**
  *frees memory allocated using functions in
  *`<kitchen-sink/backends/libc/alloc.h>`
  */
 void KS_NAME(libc_free)(void * ptr);

 KS_NAME(libc_alloc_result_t) KS_NAME(libc_alloc)(size_t const size);
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_calloc)(size_t const size);
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_alloc_aligned)(
  size_t const size,
  size_t const alignment
 );
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_calloc_aligned)(
  size_t const size,
  size_t const alignment
 );
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_realloc)(
  void * ptr,
  size_t const size
 );
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_crealloc)(
  void * ptr,
  size_t const size
 );

// /**
//  *given a pointer to a block of memory previously allocated with any of:
//  * - ks_libc_alloc_aligned
//  * - ks_libc_calloc_aligned
//  * - ks_libc_realloc_aligned
//  * - ks_libc_alloc
//  * - ks_libc_calloc
//  * - ks_libc_realloc
//  *
//  *will allocate a new block of at least `size` bytes, suitably aligned to the
//  *alignment requested in the original allocation request and memcpy all data
//  *from `ptr`, while zeroing the remaining new space. Should the requested size
//  *be smaller than the original block, only `size` bytes will be copied over
//  *from the original block. If the value of `ptr == NULL`, the behaviour is
//  *equivalent to `ks_calloc_aligned(dst, size, alignment)`
//  *
//  *on failure, the original memory block provided will remain in-tact
//  */
// KS_NAME(libc_alloc_result_t) KS_NAME(libc_realloc_aligned)(
//  void * ptr,
//  size_t const size
// );
#endif //KS_BACKEND_LIB == KS_BACKEND_LIB_LIBC
