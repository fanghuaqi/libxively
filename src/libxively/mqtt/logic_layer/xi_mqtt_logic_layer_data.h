// Copyright (c) 2003-2014, LogMeIn, Inc. All rights reserved.
// This is part of Xively C library, it is under the BSD 3-Clause license.

#ifndef __MQTT_LOGIC_LAYER_DATA_H__
#define __MQTT_LOGIC_LAYER_DATA_H__

#include "parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    // here we are going to store the mapping of the
    // handle functions versus the subscribed topics
    // so it's easy for the user to register his callback
    // for each of the subscribed topics
} xi_mqtt_logic_layer_data_t;

#ifdef __cplusplus
}
#endif

#endif // __MQTT_LOGIC_LAYER_DATA_H__