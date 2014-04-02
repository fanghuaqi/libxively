// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_LAYER_API_H__
#define __XI_LAYER_API_H__

#include "xi_layer.h"

#if defined( XI_MQTT_ENABLED ) && defined( XI_NOB_ENABLED )
    #include "xi_event_dispatcher_api.h"
    #include "xi_event_dispatcher_global_instance.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

#define CON_SELF( context ) ( ( layer_connectivity_t* ) context )->self

#ifdef XI_DEBUG_LAYER_API
#define LAYER_LOCAL_INSTANCE( layer_name_instance, layer_interface, layer_type_id, user_data )\
    layer_t layer_name_instance                     = { layer_interface, { 0, 0, 0 }, layer_type_id, ( void* ) user_data, { 0, 0, 0, 0 }, { 0, 0, 0, 0, 0, 0 } };\
    INIT_INSTANCE( layer_name_instance )
#define INIT_INSTANCE( instance )\
    instance.layer_connection.self                  = &instance;\
    instance.debug_info.debug_line_init             = __LINE__;\
    instance.debug_info.debug_file_init             = __FILE__;
#else
#define LAYER_LOCAL_INSTANCE( layer_name_instance, layer_interface, layer_type_id, user_data )\
    layer_t   layer_name_instance                   = { layer_interface, { 0, 0, 0 }, layer_type_id, ( void* ) user_data, { 0, 0, 0, 0 } };\
    INIT_INSTANCE( layer_name_instance )
#define INIT_INSTANCE( instance )\
    instance.layer_connection.self                  = &instance;
#endif

#ifdef XI_DEBUG_LAYER_API
#define CONNECT_LAYERS( lp_i, ln_i )\
    ln_i->layer_connection.prev             = lp_i;\
    lp_i->layer_connection.next             = ln_i;\
    lp_i->debug_info.debug_line_connect     = __LINE__;\
    lp_i->debug_info.debug_file_connect     = __FILE__;\
    ln_i->debug_info.debug_line_connect     = __LINE__;\
    ln_i->debug_info.debug_file_connect     = __FILE__;
#else
#define CONNECT_LAYERS( lp_i, ln_i )\
    ln_i->layer_connection.prev  = lp_i;\
    lp_i->layer_connection.next  = ln_i
#endif

#ifdef XI_DEBUG_LAYER_API
#define DISCONNECT_LAYERS( lp_i, ln_i )\
    ln_i->layer_connection.prev             = 0;\
    lp_i->layer_connection.next             = 0;\
    lp_i->debug_info.debug_line_connect     = __LINE__;\
    lp_i->debug_info.debug_file_connect     = __FILE__;\
    ln_i->debug_info.debug_line_connect     = __LINE__;\
    ln_i->debug_info.debug_file_connect     = __FILE__;
#else
#define DISCONNECT_LAYERS( lp_i, ln_i )\
    ln_i->layer_connection.prev  = 0;\
    lp_i->layer_connection.next  = 0
#endif


#define LAYER_GET_CONTEXT_PTR( instance )\
    &instance->layer_connection

#ifdef XI_DEBUG_LAYER_API
#define SET_DEBUG_INFO_ON( layer, context )\
    context->layer_connection.layer->debug_info.debug_line_last_call = __LINE__;\
    context->layer_connection.layer->debug_info.debug_file_last_call = __FILE__;
#endif

#if defined( XI_MQTT_ENABLED ) && defined( XI_NOB_ENABLED )
    #ifdef XI_DEBUG_LAYER_API
        #define CALL_ON2( layer, target, context, data, state ) \
            ({ \
                SET_DEBUG_INFO_ON( layer, context ); \
                xi_evtd_handle_t handle = { \
                      XI_EVTD_HANDLE_3_ID \
                    , .handlers.h3 = { \
                          context->layer_connection.layer->layer_functions->target \
                        , ( void* ) &context->layer_connection.layer->layer_connection \
                        , data \
                        , state } }; \
                xi_evtd_continue( xi_evtd_instance, handle, 0 ); \
            })
    #else
    #define CALL_ON2( layer, target, context, data, state )\
        ({ \
            xi_evtd_handle_t handle = { \
                  XI_EVTD_HANDLE_3_ID \
                , .handlers.h3 = { \
                      context->layer_connection.layer->layer_functions->target \
                    , ( void* ) &context->layer_connection.layer->layer_connection \
                    , data \
                    , state } }; \
            xi_evtd_continue( xi_evtd_instance, handle, 0 ); \
        })
    #endif
#else
    #ifdef XI_DEBUG_LAYER_API
    #define CALL_ON2( layer, target, context, data, state )\
        context->layer_connection.layer->layer_functions->target( &context->layer_connection.layer->layer_connection, data, state );\
        SET_DEBUG_INFO_ON( layer, context )
    #else
    #define CALL_ON2( layer, target, context, data, state )\
        context->layer_connection.layer->layer_functions->target( &context->layer_connection.layer->layer_connection, data, state )
    #endif
#endif

// ON_DEMAND
#define CALL_ON_SELF_DATA_READY( context, data, state )\
    CALL_ON2( self, data_ready, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_DATA_READY( context, data, state )\
    CALL_ON2( next, data_ready, CON_SELF( context ), data, state )

#define CALL_ON_PREV_DATA_READY( context, data, state )\
    CALL_ON2( prev, data_ready, CON_SELF( context ), data, state )

// ON_DATA_READY
#define CALL_ON_SELF_ON_DATA_READY( context, data, state )\
    CALL_ON2( self, on_data_ready, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_ON_DATA_READY( context, data, state )\
    CALL_ON2( next, on_data_ready, CON_SELF( context ), data, state )

#define CALL_ON_PREV_ON_DATA_READY( context, data, state )\
    CALL_ON2( prev, on_data_ready, CON_SELF( context ), data, state )

// CLOSE
#define CALL_ON_SELF_CLOSE( context, data, state )\
    CALL_ON2( self, close, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_CLOSE( context, data, state )\
    CALL_ON2( next, close, CON_SELF( context ), data, state )

#define CALL_ON_PREV_CLOSE( context, data, state )\
    CALL_ON2( prev, close, CON_SELF( context ), data, state )

// ON_CLOSE
#define CALL_ON_SELF_ON_CLOSE( context, data, state )\
    CALL_ON2( self, on_close, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_ON_CLOSE( context, data, state )\
    CALL_ON2( next, on_close, CON_SELF( context ), data, state )

#define CALL_ON_PREV_ON_CLOSE( context, data, state )\
    CALL_ON2( prev, on_close, CON_SELF( context ), data, state )

// INIT
#define CALL_ON_SELF_INIT( context, data, state )\
    CALL_ON2( self, init, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_INIT( context, data, state )\
    CALL_ON2( next, init, CON_SELF( context ), data, state )

#define CALL_ON_PREV_INIT( context, data, state )\
    CALL_ON2( prev, init, CON_SELF( context ), data, state )

// CONNECT
#define CALL_ON_SELF_CONNECT( context, data, state )\
    CALL_ON2( self, connect, CON_SELF( context ), data, state )

#define CALL_ON_NEXT_CONNECT( context, data, state )\
    CALL_ON2( next, connect, CON_SELF( context ), data, state )

#define CALL_ON_PREV_CONNECT( context, data, state )\
    CALL_ON2( prev, connect, CON_SELF( context ), data, state )

#ifdef __cplusplus
}
#endif

#endif // __XI_LAYER_API_H__
