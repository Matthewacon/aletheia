#pragma once

#include <kitchen-sink/macros.h>

/**
 *internal kitchen-sink-errorless allocation functions
 *
 *NOTE: all internal kitchen-sink headers do not use the
 *`<kitchen-sink/error.h>` pattern as they are required in the implementation
 *of said error handling
 */

//discourage use outside of internal implementations
#ifndef KS_INTERNAL
 #error \
  "Do not include this header directly, instead include <kitchen-sink/alloc.h>"
#endif

/*TODO:
 * - docs
 */

typedef struct {
 char const * error;
 void * result;
} KS_NAME(internal_alloc_result_t);

void KS_NAME(internal_alloc_free)(void * ptr);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc)(size_t const size);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_zero)(
 size_t const size
);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_aligned)(
 size_t const size,
 size_t const alignment
);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_aligned_zero)(
 size_t const size,
 size_t const alignment
);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_realloc)(
 void * ptr,
 size_t const size
);
KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_realloc_zero)(
 void * ptr,
 size_t const size
);
