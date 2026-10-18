#ifndef __TEMPESP_DATA_H__
#define __TEMPESP_DATA_H__

#include <stdbool.h>
#include <stdint.h>

enum relay_state_t
{
    NO_RELAY = 0,
    RELAY_OPEN = 1,
    RELAY_CLOSED = 2
};

struct meas_t
{
    int8_t  integer_part;
    uint8_t fractional_part; //Exactly 2 decimal places (ie. 1 represents .01, 99 represents .99)
};

struct measurements_t
{
    struct meas_t temperature;
    struct meas_t humidity;
};

struct report_t
{
    char marker[4]; //Contains "tesp" marker string

    uint8_t version_major;
    uint8_t version_minor;
    uint8_t instance_id;
    bool    measurements_present;

    struct measurements_t measurements;

    uint8_t relay_state;
    uint8_t battery_level;
    uint8_t seq_nr;
    uint8_t checksum;
};

#endif