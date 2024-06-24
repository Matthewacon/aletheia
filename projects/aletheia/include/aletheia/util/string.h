#pragma once

#include <stdarg.h>

//TODO: fix error handling here

//formats a string into an appropriately sized allocated buffer
char * vstring_format(char const * fmt, va_list args);

/**
 *formats a string into an appropriately sized allocated buffer
 *
 *NOTE: shim to `vstring_format`
 */
char * string_format(char const * fmt, ...);
