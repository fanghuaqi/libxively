#ifndef __XI_HEAP_TYPEDEFS_H__
#define __XI_HEAP_TYPEDEFS_H__

#include <stdint.h>

#ifndef XI_HEAP_CUSTOM_TYPES

//
typedef uint32_t xi_heap_key_type_t;

//
typedef uint8_t xi_heap_index_type_t;

#ifndef XI_HEAP_VALUE_TYPE
typedef void* xi_heap_type_t;
#endif

#else

#include "xi_heap_custom_types.i"

#endif

#endif // __XI_HEAP_TYPEDEFS_H__