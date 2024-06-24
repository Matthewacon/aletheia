/*unused declaration to prevent empty translation unit when this backend is
 *disabled
 */
static void unused(void) {}

//disable translation unit if libc is not the active backend
#include <kitchen-sink/backends/backends.h>
#if KS_BACKEND_LIB == KS_BACKEND_LIB_LIBC
 #define KS_INTERNAL
 #include <kitchen-sink/backends/internal/alloc.h>
 #include <kitchen-sink/backends/libc/alloc.h>
 #undef KS_INTERNAL

 #include <stdlib.h>
 #include <stdbool.h>
 #include <errno.h>
 #include <string.h>

 /*for convenience*/
 #define KS_MAX(a, b) (a > b ? a : b)

 /*for convenience*/
 #define KS_MIN(a, b) (a > b ? b : a)

 /*NOTE: `<kitchen-sink/backends/libc/alloc.h>` implementations*/

 /**
  *metadata struct placed in the first suitably aligned block before the
  *user-aligned block. The value of `size` and `alignment` are the
  *user-requested values
  *
  *NOTE: internal struct
  */
 typedef struct {
  size_t
   size,
   alignment,
   offset_from_start;
 } KS_NAME(libc_meta_t);

 /**
  *requirements for an allocation
  *
  *NOTE: internal struct
  */
 typedef struct {
  size_t
   user_size,
   user_alignment,
   new_size,
   new_alignment;
 } KS_NAME(libc_allocation_t);

 /**
  *given the user-requested size and alignment, compute the new allocation size
  *and alignment requirements. Accounts for size and alignment requirements
  *of the metadata structure, `libc_layout_t`, and assumes that the subsequent
  *allocation is unaligned (meaning the computed size will account for enough
  *space to manually suitably align both the metadata stuct, `libc_layout_t`,
  *as well as the user-aligned block).
  *
  *NOTE: internal function
  */
 static void KS_NAME(libc_compute_allocation)(
  KS_NAME(libc_allocation_t) * allocation
 ) {
  //compute minimum size for unaligned allocation
  allocation->new_size =
   //metadata block
   sizeof(KS_NAME(libc_meta_t))
   + KS_ALIGNOF(KS_NAME(libc_meta_t))
   //user block
   + allocation->user_size
   + allocation->user_alignment;

  //compute minimum alignment for aligned allocation
  allocation->new_alignment = KS_MAX(
   KS_ALIGNOF(KS_NAME(libc_meta_t)),
   allocation->user_alignment
  );
 }

 /**
  *descriptor contianing the metadata-aligned and user-aligned blocks within
  *a given allocation
  *
  *NOTE: internal struct
  */
 typedef struct {
  void
   * meta_block,
   * user_block;
 } KS_NAME(libc_layout_t);

 /**
  *computes the addresses, relative to `base`, of the metadata-aligned and
  *user-aligned blocks
  *
  *NOTE: internal function
  */
 static KS_NAME(libc_layout_t) KS_NAME(libc_compute_layout)(
  KS_NAME(libc_allocation_t) const * allocation,
  void * base
 ) {
  //compute the aligned user block address
  void * const user_block_addr = KS_NAME(to_aligned_address)(
   (char *)base
    + sizeof(KS_NAME(libc_meta_t))
    + KS_ALIGNOF(libc_meta_t),
   allocation->user_alignment,
   KS_NAME(ALIGN_NEXT)
  );

  //compute the aligned metadata block address
  void * const meta_block_addr = KS_NAME(to_aligned_address)(
   (char *)user_block_addr - sizeof(KS_NAME(libc_meta_t)),
   KS_ALIGNOF(KS_NAME(libc_meta_t)),
   KS_NAME(ALIGN_LAST)
  );

  return (KS_NAME(libc_layout_t)) {
   .meta_block = meta_block_addr,
   .user_block = user_block_addr,
  };
 }

 /**
  *given a pointer to an aligned user-block, compute the address of the
  *preceeding metadata block
  *
  *NOTE: internal function
  */
 static KS_NAME(libc_meta_t) * KS_NAME(libc_get_meta)(void * user_block) {
  return (KS_NAME(libc_mata_t) *)KS_NAME(to_aligned_address)(
   (char *)user_block - sizeof(KS_NAME(libc_meta_t)),
   KS_ALIGNOF(KS_NAME(libc_meta_t)),
   KS_NAME(ALIGN_LAST)
  );
 }

 /**
  *given a pointer to an aligned user-block, compute the base address of the
  *allocation
  *
  *NOTE: internal function
  */
 static void * KS_NAME(libc_get_base)(void * user_block) {
  KS_NAME(libc_meta_t) const * meta = KS_NAME(libc_get_meta)(user_block);
  return (char *)meta - meta->offset_from_start;
 }

 /**
  *initializes the `libc_meta_t` instance at the `layout->meta_block` with the
  *user-requested size and alignment, as well as the offset of
  *`layout->meta_block` from `base`
  *
  *NOTE: internal function
  */
 static void KS_NAME(libc_initialize_meta_block)(
  KS_NAME(libc_allocation_t) const * allocation,
  KS_NAME(libc_layout_t) const * layout,
  void * base
 ) {
  (KS_NAME(libc_meta_t) *)layout->meta_block = (KS_NAME(libc_meta_t)) {
   .size = allocation->user_size,
   .alignment = allocation->user_alignment,
   .offset_from_start = (size_t)layout->meta_block - (size_t)base,
  };
 }

 /**
  *allocates a block of memory contianing space for metadata about the
  *allocation and space for the user-requested data, both suitably aligned
  *
  *NOTE: cannot use `aligned_alloc()` here since it is not in C99
  *
  *NOTE: internal function
  */
 static KS_NAME(libc_alloc_result_t) KS_NAME(libc_internal_alloc)(
  size_t const size,
  size_t const alignment,
  bool const zero
 ) {
  //if the requested alignment is zero, return error
  if (!alignment) {
   return (KS_NAME(libc_alloc_result_t)) {
    .error = KS_NAME_STR(libc_alloc_aligned) "(): The minimum alignment is 1!",
    .result = NULL,
   };
  }

  //compute allocation requirements
  KS_NAME(libc_allocation_t) allocation = {
   .user_size = size,
   .user_alignment = 1,
   .new_size = 0,
   .new_alignemnt = 0,
  };
  KS_NAME(libc_compute_allocation)(&allocation);

  //perform allocation
  errno = 0;
  void * base;
  if (zero) {
   base = calloc(1, allocation->new_size);
  } else {
   base = malloc(allocation->new_size);
  }

  //if allocation failed, provide libc errno string
  if (errno) {
   return (KS_NAME(libc_alloc_result_t)) {
    .code = errno,
    .error = strerror(errno),
    .result = NULL,
   };
  }

  //if errno is not set but we did not receive memory, return error
  if (!base) {
   return (KS_NAME(libc_alloc_result_t)) {
    .code = 0,
    .error = KS_NAME_STR(libc_internal_alloc) "(): `errno` was not set but "
     "`malloc()`/`calloc()` did not produce anything either!",
    .result = NULL,
   };
  }

  //initialize metadata block in allocated memory
  KS_NAME(libc_layout_t) layout = KS_NAME(libc_compute_layout)(
   &allocation,
   base
  );
  KS_NAME(libc_initialize_meta_block)(&allocation, &layout, base);

  //return user block
  return (KS_NAME(libc_alloc_result_t)) {
   .error = NULL,
   .result = layout->user_block,
  };
 }

 /**
  *reallocates a block of memory contianing space for metadata about the
  *allocation and space for the user-requested data, both suitably aligned.
  *Existing user data will be moved to a suitably-aligned address, based on the
  *user-requested alignment requirements, specified on the initial allocation.
  *If `ptr == NULL`, will allocate a new block of memory. Preserves `ptr` in
  *the event of an error
  *
  *NOTE: internal function
  */
 static KS_NAME(libc_alloc_result_t) KS_NAME(libc_internal_realloc)(
  void * ptr,
  size_t const size,
  bool const zero
 ) {
  //if the provided pointer is `NULL`, perform initial allocation
  if (!ptr) {
   return KS_NAME(libc_internal_alloc)(size, 1, zero);
  }

  //get metadata for old allocation
  KS_NAME(libc_meta_t) old_meta = *KS_NAME(libc_get_meta)(ptr);
  void * old_user_block = ptr;
  void * old_base = KS_NAME(libc_get_base)(old_user_block);
  KS_NAME(libc_allocation_t) old_allocation = {
   .user_size = old_meta.size,
   .user_alignment = old_meta.alignment,
   .new_size = 0,
   .new_alignment = 0,
  };

  //compute new allocation requirements
  KS_NAME(libc_allocation_t) allocation = {
   .user_size = size,
   .user_alignment = old_allocation->user_alignment,
   .new_size = 0,
   .new_alignment = 0,
  };
  KS_NAME(libc_compute_allocation)(&allocation);

  //perform reallocation
  errno = 0;
  void * base = realloc(old_base, allocation->new_size);

  //if allocation failed, provide libc errno string
  if (errno) {
   return (KS_NAME(libc_alloc_result_t)) {
    .code = errno,
    .error = strerror(errno),
    .result = NULL,
   };
  }

  //if errno is not set but we did not receive memory, return error
  if (!base) {
   return (KS_NAME(libc_alloc_result_t)) {
    .code = 0,
    .error = KS_NAME_STR(libc_realloc) "(): `errno` was not set but "
     "`realloc()` did not produce anything either!",
    .result = NULL,
   };
  }

  //NOTE: all `old_*` pointers are invalidated after `realloc()` succeeds

  //compute new metadata and user block addresses
  KS_NAME(libc_layout_t) layout = KS_NAME(libc_compute_layout)(
   &allocation,
   base
  );

  /*move user block data to new aligned address
   *
   *NOTE: no need to move if the user block is suitably aligned within the
   *`realloc()`'d block
   */
  size_t const old_block_offset = (size_t)old_user_block - (size_t)old_base;
  size_t const new_block_offset = (size_t)layout.user_block - (size_t)base;
  if (old_block_offset != new_block_offset) {
   memmove(
    (char *)base + new_block_offset,
    (char *)base + old_block_offset,
    KS_MIN(old_meta.user_size, size)
   );
  }

  //zero trailing space, if requested
  if (zero && size > old_meta.user_size) {
   memset(
    (char *)layout.user_block + allocation.user_size,
    0,
    size - old_meta.user_size
   );
  }

  //initialize metadata block in allocated memory
  KS_NAME(libc_initialize_meta_block)(&allocation, &layout, base);

  //return user block
  return (KS_NAME(libc_alloc_result_t)) {
   .error = NULL,
   .result = layout->user_block,
  };

 }

 //`libc_free` implementation
 void KS_NAME(libc_free)(void * ptr) {
  free(KS_NAME(libc_get_base(ptr)));
 }

 //`libc_alloc` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_alloc)(size_t const size) {
  return KS_NAME(libc_internal_alloc)(size, 1, false);
 }

 //`libc_calloc` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_calloc)(size_t const size) {
  return KS_NAME(libc_internal_calloc)(size, 1, true);
 }

 //`libc_alloc_aligned` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_alloc_aligned)(
  size_t const size,
  size_t const alignment
 ) {
  return KS_NAME(libc_internal_alloc)(size, alignment, false);
 }

 //`libc_calloc_aligned` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_calloc_aligned)(
  size_t const size,
  size_t const alignment
 ) {
  return KS_NAME(libc_internal_alloc)(size, alignment, true);
 }

 //`libc_realloc` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_realloc)(
  void * ptr,
  size_t const size
 ) {
  return KS_NAME(libc_internal_realloc)(ptr, size, false);
 }

 //`libc_crealloc` implementation
 KS_NAME(libc_alloc_result_t) KS_NAME(libc_crealloc)(
  void * ptr,
  size_t const size
 ) {
  return KS_NAME(libc_internal_realloc)(ptr, size, true);
 }

 /*`<kitchen-sink/backends/internal/alloc.h>` implementations
  *
  *NOTE: nothing fancy going on here, just shims to `libc_*` allocation
  *functions
  */

 //`internal_alloc_free` implementation
 void KS_NAME(internal_alloc_free)(void * ptr) {
  return KS_NAME(libc_free)(ptr);
 }

 //`internal_alloc` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc)(
  size_t const size
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_alloc)(size);
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }

 //`internal_alloc_zero` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_zero)(
  size_t const size
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_calloc)(size);
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }

 //`internal_alloc_aligned` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_aligned)(
  size_t const size,
  size_t const alignment
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_alloc_aligned)(
   size,
   alignment
  );
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }

 //`internal_alloc_aligned_zero` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_aligned_zero)(
  size_t const size,
  size_t const alignment
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_calloc_aligned)(
   size,
   alignment
  );
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }

 //`internal_alloc_realloc` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_realloc)(
  void * ptr,
  size_t const size
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_realloc)(
   ptr,
   size
  );
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }

 //`internal_alloc_realloc_zero` implementation
 KS_NAME(internal_alloc_result_t) KS_NAME(internal_alloc_realloc_zero)(
  void * ptr,
  size_t const size
 ) {
  KS_NAME(libc_alloc_result_t) const result = KS_NAME(libc_crealloc)(
   ptr,
   size
  );
  return (KS_NAME(internal_alloc_result_t)) {
   .error = result.error,
   .result = result.result,
  };
 }
#endif //KS_BACKEND_LIB == KS_BACKEND_LIB_LIBC
