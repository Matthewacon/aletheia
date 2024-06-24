#pragma once

#include <kitchen-sink/macros.h>

/**
 *internal kitchen-sink-errorless allocation functions
 *
 *NOTE: headers in `<kitchen-sink/internal/*>` may not use headers from
 *`<kitchen-sink/*>`, as they are used in the implementation of those headers
 */

//discourage use outside of internal implementations
#ifndef KS_INTERNAL
 #error \
  "Do not include this header directly, instead include <kitchen-sink/alloc/alloc.h>"
#endif

/*TODO:
 * - docs
 */

typedef struct {
 char const * error;
 void * result;
} KS_NAME(internal_alloc_result_t);

typedef struct {
 void * (* KS_NAME(free))(void * ptr);

 KS_NAME(internal_alloc_result_t) (* KS_NAME(alloc))(size_t const size);

 KS_NAME(internal_alloc_result_t) (* KS_NAME(alloc_zero))(size_t const size);

 KS_NAME(internal_alloc_result_t) (* KS_NAME(alloc_aligned))(
  size_t const size,
  size_t const alignment
 );

 KS_NAME(internal_alloc_result_t) (* KS_NAME(alloc_aligned_zero))(
  size_t const size,
  size_t const alignment
 );

 KS_NAME(internal_alloc_result_t) (* KS_NAME(realloc))(
  void * ptr,
  size_t const size
 );

 KS_NAME(internal_alloc_result_t) (* KS_NAME(realloc_zero))(
  void * ptr,
  size_t const size
 );
} KS_NAME(internal_alloc_t);

/**
 *default internal allocator implementation
 */
extern KS_NAME(internal_alloc_t) KS_NAME(internal_alloc_default);
