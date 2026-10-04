#ifndef __TEMPESP_DRIVERS_H__
#define __TEMPESP_DRIVERS_H__

#include "stdbool.h"

struct temp_t
{
    unsigned short integer_part;
    unsigned int fractional_part;
};

bool initialize_sensor();

struct temp_t get_temperature_reading();

unsigned int get_humidity_reading();

void set_relay_state(bool closed);

#endif