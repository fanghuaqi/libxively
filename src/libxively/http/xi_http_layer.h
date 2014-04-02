// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __XI_HTTP_LAYER_H__
#define __XI_HTTP_LAYER_H__

#include "xi_layer.h"
#include "xi_http_layer_input.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t http_layer_data_ready(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t http_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t http_layer_init(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t http_layer_connect(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t http_layer_close(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t http_layer_on_close(
      void* context
    , void* data
    , layer_state_t state );

const void* http_layer_data_generator_datastream_get(
      const void* input
    , short* state );

#ifdef __cplusplus
}
#endif

#endif // __XI_HTTP_LAYER_H__
