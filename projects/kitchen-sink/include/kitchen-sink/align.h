#pragma once

#include <kitchen-sink/types.h>
#include <kitchen-sink/macros.h>

/**
 *`alignof` did not exist for C99, so we have our own implementation
 *
 *TODO: do not use libc types
 */
#define KS_ALIGNOF(type) (\
 (KS_NAME(size_t))(&((struct { char unused; type t; } *)0)->t) \
)

/**
 *enum for specifying alignment direction when calling
 *`ks_to_aligned_address()`
 *
 * - ALIGN_LAST: aligns pointer to the address, on or before the provided
 *   address, that is suitably aligned to the provided alignment
 * - ALIGN_NEXT: aligns the pointer to the address, on or after the provided
 *   address, that is suitably aligned to the provided alignment
 */
typedef enum {
 KS_NAME(ALIGN_LAST) = 0,
 KS_NAME(ALIGN_NEXT) = 1
} KS_NAME(align_direction_t);

/**
 *aligns `ptr` to the previous or next address suitably aligned to `alignment`
 *
 *see `align_direction_t` for information about alignment direction
 */
void * KS_NAME(to_aligned_address)(
 void * ptr,
 KS_NAME(size_t) const alignment,
 KS_NAME(align_direction_t) const direction
);
