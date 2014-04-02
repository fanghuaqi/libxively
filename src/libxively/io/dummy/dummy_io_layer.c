// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "dummy_io_layer.h"
#include "dummy_data.h"
#include "xi_allocator.h"
#include "xi_err.h"
#include "xi_macros.h"
#include "xi_debug.h"

#include "xi_layer_api.h"
#include "xi_common.h"

layer_state_t dummy_io_layer_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t dummy_io_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t dummy_io_layer_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t dummy_io_layer_on_close(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t dummy_io_layer_init(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}

layer_state_t dummy_io_layer_connect(
      void* context
    , void* data
    , layer_state_t in_state )
{
    XI_UNUSED( context );
    XI_UNUSED( data );
    XI_UNUSED( in_state );

    return LAYER_STATE_OK;
}
