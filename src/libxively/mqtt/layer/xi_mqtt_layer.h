// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __MQTT_LAYER_H__
#define __MQTT_LAYER_H__

#include "xi_layer.h"
#include "xi_common.h"
#include "xi_mqtt_layer_data.h"

#ifdef __cplusplus
extern "C" {
#endif

layer_state_t xi_mqtt_layer_data_ready(
      void* context
    , void* data
    , layer_state_t );

layer_state_t xi_mqtt_layer_on_data_ready(
      void* context
    , void* data
    , layer_state_t );

layer_state_t xi_mqtt_layer_init(
      void* context
    , void* data
    , layer_state_t );

layer_state_t xi_mqtt_layer_connect(
      void* context
    , void* data
    , layer_state_t );

layer_state_t xi_mqtt_layer_close(
      void* context
    , void* data
    , layer_state_t state );

layer_state_t xi_mqtt_layer_on_close(
      void* context
    , void* data
    , layer_state_t state );

#ifdef __cplusplus
}
#endif

#endif //__MQTT_LAYER_H__