/*TODO:
 * - use `<kitchen-sink/backend/*>` headers instead of libc
 * - replace libc types with internal types
 */
#include <stddef.h>

#include <kitchen-sink/error.h>

/**
 *descriptor for a dynamic error with optional context information
 */
typedef struct {
 //mandatory first field to distinguish error type
 KS_NAME(error_static_ident_t) ident;
 //optional context data
 void * ctx;
 void (* ctx_free)(void * ctx);
 /*offsets, in bytes, from an instance of `error_dynamic_t` pointing to null
  *terminated strings
  */
 size_t const
  offset_message,
  offset_file,
  offset_function,
  offset_line;
 //NOTE: message buffer expected to follow instances of this struct
 //char message_data[];
} KS_NAME(error_dynamic_t);

/**
 *descriptor for an element in a stack of contiguous errors
 */
typedef struct {
 /*NOTE: must be the first members in this struct in order to properly address
  *the `ident` field of `static_error` and `dynamic_error`
  */
 union {
  //NOTE: overlaps with `ident` in `static_error` and `dynamic_error`
  KS_NAME(error_ident_t) ident;
  KS_ERROR_STATIC_DECL(error_static);
  KS_NAME(dynamic_error_t) error_dynamic;
 } error;
 /*offsets, in bytes, from an instance of `error_stack_frame_t` to the previous
  *or next `error_stack_frame_t` instance
  *
  *NOTE: 1-indexed, a value of `0` indicates no previous or next frame,
  *respectively
  */
 size_t const
  offset_prev,
  offset_next;
} KS_NAME(error_stack_frame_t);

/**
 *descriptor for a stack of errors
 */
typedef struct {
 //mandatory first field to distinguish error type
 KS_NAME(error_ident_t) ident;
 size_t
  capacity,
  size,
  count;
 /*NOTE: contiguous array of frames immediately following an instance of
  *`error_stack_t`
  */
 //KS_NAME(error_stack_frame_t) errors[];
} KS_NAME(error_stack_t);

/**
 *returns the next `alignment` aligned address after `ptr`; if `ptr` is
 *suitably aligned, returns `ptr`
 *
 *NOTE: internal function
 */
static void * KS_NAME(_next_aligned_address)(
 void * ptr,
 size_t const alignment
) {
 size_t const ptr_value = (size_t)ptr;
 if (ptr_value % alignment != 0) {
  return (void *)(alignment * ((ptr_value / alignment) + 1));
 }
 return ptr;
}

/**
 *TODO:
 *
 *NOTE: internal function
 */
static void KS_NAME(_error_realloc_contiguous)(
 KS_NAME(error_t) * error,
 size_t const new_size
) {
 (void)error;
}

/**
 *frees all user-supplied error contexts, in reverse order
 *
 *NOTE: internal function
 */
static void KS_NAME(_error_free_contexts)(KS_NAME(error_t) * error) {
 KS_NAME(error_ident_t) * const ident = (KS_NAME(error_ident_t) *)*error;

 //free the user-supplied context of a dynamic frame, if present
 if (*ident == KS_NAME(ERROR_DYNAMIC)) {
  KS_NAME(error_dynamic_t) * dynamic = (KS_NAME(error_dynamic_t) *)*error;
  //if the user did not provide a `ctx_free` callback, do nothing
  if (!dynamic->ctx_free) {
   return;
  }
  void * ctx = dynamic->ctx;
  void (*ctx_free)(void * ctx) = dynamic->ctx_free;
  dynamic->ctx = NULL;
  dynamic->ctx_free = NULL;
  ctx_free(ctx);
 }

 //free all dynamic user-supplied contexts, in reverse order
 if (*ident == KS_NAME(ERROR_STACK)) {
  KS_NAME(error_stack_t) * stack = (KS_NAME(error_stack_t) *)*error;
  void * ctxs[stack->count] = {0};
  void (* ctx_frees[stack->count])(void * ctx) = {0};

  //collect contexts and context free functions
  {
   size_t index = 0;
   char * raw = ((char *)(void *)stack) + sizeof(KS_NAME(error_stack_t));
   while (index < stack->count) {
    //get next frame ptr
    raw = KS_NAME(_next_aligned_address)(raw, KS_ALIGNOF(error_stack_frame_t));
    KS_NAME(error_stack_frame_t) * frame = (KS_NAME(error_stack_frame_t) *)raw;

    /*NOTE: do not need to handle static frames since they do not contain
     *contexts
     */

    //handle dynamic frames
    if (*frame->ident == KS_NAME(ERROR_DYNAMIC)) {
     KS_NAME(error_dynamic_t) * dynamic = frame->error_dynamic;
     //zero fields and push contexts
     void * ctx = dynamic->ctx;
     void (* ctx_free)(void * ctx) = dynamic->ctx_free;
     dynamic->ctx = NULL;
     dynamic->ctx_free = NULL;
     ctxs[index] = ctx;
     ctx_frees[index] = ctx_free;
    }

    //increment `raw` to point to next frame (alignment handled above)
    raw += dynamic->offset_next;
    //increment counter
    index++;
   }
  }

  //free contexts in reverse
  for (size_t i = stack->count; i > 0; i--) {
   if (!ctx_frees[i - 1]) {
    continue;
   }
   ctx_frees[i - 1](ctxs[i - 1]);
  }
 }

 //NOTE: nothing to do for `ERROR_STATIC` since they do not have context data
}

//`error_free` implementation
void KS_NAME(error_free)(KS_NAME(error_t) * error) {
 //do nothing if the error is already gone
 if (!error || !*error) {
  return;
 }

 //zero user error before we do anything
 void * underlying_error = *error;
 *error = NULL;

 KS_NAME(error_ident_t) * const ident = (KS_NAME(error_ident_t) *)underlying_error;

 //static errors are never allocated, so do nothing
 if (*ident == KS_NAME(ERROR_STATIC)) {
  return;
 }

 //free all user-supplied dynamic error contexts
 KS_NAME(_error_free_contexts)((KS_NAME(error_t) *)&underlying_error);

 //free contiguous underlying error buffer
 free(underlying_error);
}

//TODO:
void KS_NAME(error_new)(KS_NAME(error_t) * error, char const * message) {
 (void)error;
 (void)message;
 //clear previous error, if any
 KS_NAME(error_free)(error);

 //
}

//TODO:
void KS_NAME(error_new_with_context)(
 KS_NAME(error_t) * error,
 char const * message,
 void * ctx,
 void (* ctx_free)(void * ctx)
) {
 (void)error;
 (void)message;
 (void)ctx;
 (void)ctx_free;
}



//TODO: all other functions
