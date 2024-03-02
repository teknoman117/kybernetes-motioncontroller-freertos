#pragma once

#include <stddef.h>
#include <stdint.h>

typedef struct i2c_msg {
    uint8_t *data;
    uint8_t len;
    uint8_t address;
} i2c_msg_t ;

typedef struct _i2c_transfer {
    i2c_msg_t *msgs;
    uint8_t n;
} i2c_transfer_t ;
