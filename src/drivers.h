#ifndef __TEMPESP_DRIVERS_H__
#define __TEMPESP_DRIVERS_H__

#include "stdbool.h"

struct meas_t
{
    int8_t  integer_part;
    uint8_t fractional_part; //exactly 2 decimal places
};

struct measurements_t
{
    struct meas_t temperature;
    struct meas_t humidity;
};

bool initialize_sensor();

bool get_measurements(struct measurements_t* result);

void set_relay_state(bool closed);

#endif