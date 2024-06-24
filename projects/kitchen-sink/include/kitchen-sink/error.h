#pragma once

#include <kitchen-sink/macros.h>

/**
 *utility library for handling errors
 *
 *NOTE: cannot use other kitchen-sink headers in this header or its
 *implementation as it is used in the implementation of all other kitchen-sink
 *headers and features
 */

/*TODO:
 * - macros for declaring and defining static errors
 * - error functions with kitchen-sink string types
 * - make all error functions do nothing on receiving `NULL` as the error pointer
 * - document everything
 */

typedef void * KS_NAME(error_t);

typedef enum {
 KS_NAME(ERROR_STATIC) = 1,
 KS_NAME(ERROR_DYNAMIC),
 KS_NAME(ERROR_STACK),
} KS_NAME(error_ident_t);

void KS_NAME(error_free)(KS_NAME(error_t) * error);
void KS_NAME(error_new)(KS_NAME(error_t) * error, char const * message);
void KS_NAME(error_new_with_context)(
 KS_NAME(error_t) * error,
 char const * message,
 void * ctx,
 void (* ctx_free)(void * ctx)
);
void KS_NAME(error_push)(KS_NAME(error_t) * error, KS_NAME(error_t) new_error);
void KS_NAME(error_pop)(KS_NAME(error_t) * error, KS_NAME(error_t) * dst);
void KS_NAME(error_peek)(KS_NAME(error_t) * error, KS_NAME(error_t) * dst);
void KS_NAME(error_details)(
 KS_NAME(error_t) * error,
 char const ** message,
 char const ** file,
 char const ** function,
 size_t ** line,
 void ** ctx
);

//TODO: maybe combine these into a single function with result ptrs?
void KS_NAME(error_get_type)(KS_NAME(error_t) * error, KS_NAME(error_t) * type);

void KS_NAME(error_to_string)(KS_NAME(error_t) * error, char const * message);

//TODO: macros
#define KS_ERROR_STATIC_TYPE \
struct { \
 /*mandatory first field to distinguish error type*/ \
 KS_NAME(error_ident_t) ident; \
 char const * message; \
}

#define KS_ERROR_STATIC_DECL(name) \
KS_ERROR_STATIC_TYPE name

#define KS_ERROR_STATIC_DEFN(name, message) \
KS_ERROR_STATIC_DECL(name) = { \
 .ident = KS_NAME(ERROR_STATIC), \
 .##message = message, \
}
