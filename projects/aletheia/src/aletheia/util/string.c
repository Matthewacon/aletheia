#include <aletheia/util/string.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

//TODO: fix error handling here

//`vstring_format` implementation
char * vstring_format(char const * fmt, va_list l) {
 //determine length of formatted string
 va_list copy;
 va_copy(copy, l);
 size_t const length = vsnprintf(NULL, 0, fmt, copy) + 1;
 va_end(copy);

 //allocate buffer and actually format string
 char * result = malloc(sizeof(char) * length);
 if (!result) {
  return NULL;
 }
 vsnprintf(result, length, fmt, l);

 return result;
}

//`string_format` implementation
char * string_format(char const * fmt, ...) {
 va_list l;
 va_start(l, fmt);
 char * result = vstring_format(fmt, l);
 va_end(l);
 return result;
}
