// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __DUMMY_IO_LAYER_H__
#define __DUMMY_IO_LAYER_H__

#include "xi_layer.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t dummy_io_layer_data_ready(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint );

layer_state_t dummy_io_layer_on_data_ready(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint );

layer_state_t dummy_io_layer_close(
    struct layer_connectivity_s* context );

layer_state_t dummy_io_layer_on_close(
    struct layer_connectivity_s* context );

layer_state_t dummy_io_layer_init(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint );

layer_state_t dummy_io_layer_connect(
      struct layer_connectivity_s* context
    , const void* data
    , const layer_hint_t hint );

#ifdef __cplusplus
}
#endif

#endif // __DUMMY_IO_LAYER_H__
