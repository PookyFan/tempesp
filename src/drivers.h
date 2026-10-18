#ifndef __TEMPESP_DRIVERS_H__
#define __TEMPESP_DRIVERS_H__

#include "stdbool.h"
#include "data.h"

bool initialize_sensor();

bool get_measurements(struct measurements_t* result);

void set_relay_state(bool closed);

#endif