#pragma once

/**
 *TODO: DOC 
 */

#include <kitchen-sink/macros.h>
#include <kitchen-sink/static-assert.h>

#define KS_INTERNAL
 #include <kitchen-sink/internal/backends.h>
#undef KS_INTERNAL

#if KS_BACKEND_LIB == KS_BACKEND_LIB_FREESTANDING
 #error "The <kitchen-sink/types.h> header is unimplemented for freestanding builds!"
#elif KS_BACKEND_LIB == KS_BACKEND_LIB_LIBC
 #include <kitchen-sink/backends/libc/types.h>
#else
 #error "Unknown value specified for KS_BACKEND_LIB"
#endif

/*TODO: static assertions to ensure types are defined
 * uint8_t, int8_t
 * uint16_t, int16_t
 * uint32_t, int32_t
 * uint64_t, int64_t
 * size_t (sizeof(size_t) == sizeof(void *))
 * float8_t
 * float16_t
 * float32_t
 * float64_t
 */

