#pragma once

//symbol name substitution macro
#ifndef KS_NAME_PREFIX
 #define KS_NAME_PREFIX ks_
#endif

/**
 *adds a layer of indirection before invoking the macro `m` so macro
 *definitions can expand
 */
#define KS_INDIRECT(m, ...) m(__VA_ARGS__)

/**
 *expands to `value` as a string
 */
#define KS_STR(value) #value

//internal
#define _KS_NAME(prefix, name) prefix##name

/**
 *expands to the expected symbol name for a global symbol in kitchen-sink
 */
#define KS_NAME(name) KS_INDIRECT(_KS_NAME, KS_NAME_PREFIX, name)

/**
 *same as `KS_NAME(...)` but expands to a string
 */
#define KS_NAME_STR(name) KS_INDIRECT(KS_STR, KS_NAME_PREFIX) KS_STR(name)
