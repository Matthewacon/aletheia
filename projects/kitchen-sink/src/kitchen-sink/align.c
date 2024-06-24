#include <kitchen-sink/macros.h>

//`to_aligned_address` implementation
void * KS_NAME(to_aligned_address)(
 void * ptr,
 size_t const alignment,
 KS_NAME(align_direction_t) const direction
) {
 size_t const ptr_value = (size_t)ptr;
 if (ptr_value % alignment != 0) {
  return (void *)(alignment * ((ptr_value / alignment) + direction));
 }
 return ptr;
}
