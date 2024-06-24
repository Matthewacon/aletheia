#pragma once

#include <kitchen-sink/macros.h>
#include <kitchen-sink/error.h>

/**
 *TODO:
 *
 *this header contains the definitions for implementing instanced allocators
 */

/**
 *TODO: DOC
 */
typedef struct {
 //TODO: DOC
 void (* KS_NAME(free))(void * ptr);

 //TODO: DOC
 void * (* KS_NAME(alloc))(KS_NAME(error_t) * error, size_t const size);

 //TODO: DOC
 void * (* KS_NAME(alloc_aligned))(
  KS_NAME(error_t) * error,
  size_t const size,
  size_t const alignment
 );

 //TODO: DOC
 void * (* KS_NAME(alloc_zero))(KS_NAME(error_t) * error, size_t const size);

 //TODO: DOC
 void * (* KS_NAME(alloc_aligned_zero))(
  KS_NAME(error_t) * error,
  size_t const size,
  size_t const alignment
 );

 //TODO: DOC
 void * (* KS_NAME(realloc))(
  KS_NAME(error_t) * error,
  void * ptr,
  size_t const new_size
 );

 //TODO: DOC
 void * (* KS_NAME(realloc_zero))(
  KS_NAME(error_t) * error,
  void * ptr,
  size_t const new_size
 );
} KS_NAME(alloc_t);

//TODO: default allocator
extern KS_NAME(alloc_t) const KS_NAME(alloc_default);

//TODO: convenience macros
#define ks_alloc_ez(alloc, error, type) \
(type *)alloc->##alloc(error, sizeof(type))

#define ks_alloc_aligned_ez(alloc, error, type) \
(type *)alloc->##alloc_aligned(error, sizeof(type), KS_ALIGNOF(type))

#define ks_alloc_zero_ez(alloc, error, type) \
(type *)alloc->##alloc_zero(error, sizeof(type))

#define ks_alloc_aligned_zero_ez(alloc, error, type) \
(type *)alloc->##alloc_aligned_zero(error, sizeof(type), KS_ALIGNOF(type))

#define ks_realloc_ez(alloc, error, type, ptr, new_size) \
(type *)alloc->##realloc(error, ptr, new_size)

#define ks_realloc_zero_ez(alloc, error, type, ptr, new_size) \
(type *)alloc->##realloc_zero(error, ptr, new_size)
