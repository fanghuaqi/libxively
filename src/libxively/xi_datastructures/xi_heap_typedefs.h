#ifndef __XI_HEAP_TYPEDEFS_H__
#define __XI_HEAP_TYPEDEFS_H__

#include <stdint.h>

#ifndef XI_HEAP_CUSTOM_TYPES

#include "xi_event_handler.h"

//
typedef uint32_t            xi_heap_key_type_t;
typedef uint8_t             xi_heap_index_type_t;
typedef xi_evtd_handle_t    xi_heap_value_type_t;

#else

#include "xi_heap_custom_types.i"

#endif

#endif // __XI_HEAP_TYPEDEFS_H__