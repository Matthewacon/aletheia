#pragma once

#include <kitchen-sink/macros.h>
#include <kitchen-sink/error.h>

/*TODO:
 * - replace libc types
 * - add static errors
 * - docs
 * - add feature macro to disable convenience macro definitions?
 */

void KS_NAME(alloc_free)(void * ptr);

void * KS_NAME(alloc)(KS_NAME(error_t) * error, size_t const size);
#define ks_alloc_ez(error, type) \
(type *)KS_NAME(alloc)(error, sizeof(type))

void * KS_NAME(alloc_aligned)(
 KS_NAME(error_t) * error,
 size_t const size,
 size_t const alignment
);
#define ks_alloc_aligned_ez(error, type) \
(type *)KS_NAME(alloc_aligned)(error, sizeof(type), KS_ALIGNOF(type))

void * KS_NAME(alloc_zero)(KS_NAME(error_t) * error, size_t const size);
#define ks_alloc_zero_ez(error, type) \
(type *)KS_NAME(alloc_zero)(error, sizeof(type))

void * KS_NAME(alloc_aligned_zero)(
 KS_NAME(error_t) * error,
 size_t const size,
 size_t const alignment
);
#define ks_alloc_aligned_zero_ez(error, type) \
(type *)KS_NAME(alloc_aligned_zero)(error, sizeof(type), KS_ALIGNOF(type))

void * KS_NAME(alloc_realloc)(
 KS_NAME(error_t) * error,
 void * ptr,
 size_t const new_size
);
#define ks_alloc_realloc_ez(error, type, ptr, size) \
(type *)KS_NAME(alloc_realloc)(error, ptr, size)

void * KS_NAME(alloc_realloc_zero)(
 KS_NAME(error_t) * error,
 void * ptr,
 size_t const new_size
);
#define ks_alloc_realloc_zero_ez(error, type, ptr, size) \
(type *)KS_NAME(alloc_realloc_zero)(error, ptr, size)
