#include "espressif/esp_common.h"

#include "i2c/i2c.h"
#include "utils.h"
#include "drivers.h"

#define BUS     0
#define SCL_PIN 5
#define SDA_PIN 4

#define MANUFACTURER_ID_REG 0xFE
#define DEVICE_ID_REG       0xFF

//Hardware IDs have their bytes swapped, as they are received
//on I2c with MSB first, while ESP8266's MCU is little-endian
#define MANUFACTURER_ID_VAL 0x4954
#define DEVICE_ID_VAL       0x5010 

static uint8_t sensor_i2c_addr = 0x40;

bool initialize_sensor()
{
    uint8_t m_id[2] __attribute__((aligned(2))) = {MANUFACTURER_ID_REG};
    uint8_t d_id[2] __attribute__((aligned(2))) = {DEVICE_ID_REG};
    if(i2c_init(BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K))
    {
        printf("Failed to initialize I2C bus\n");
        return false;
    }

    if( i2c_slave_read(BUS, sensor_i2c_addr, m_id, m_id, ARRAY_SIZE(m_id))
     || i2c_slave_read(BUS, sensor_i2c_addr, d_id, d_id, ARRAY_SIZE(d_id)))
    {
        printf("Failed to read sensor IDs\n");
        return false;
    }

    uint16_t* manufacturer_id = (uint16_t*)m_id;
    uint16_t* device_id = (uint16_t*)d_id;
    if(*manufacturer_id != MANUFACTURER_ID_VAL || *device_id != DEVICE_ID_VAL)
    {
        printf("Invalid ID: manufacturer = %x, device = %x\n", *manufacturer_id, *device_id);
        return false;
    }

    return true;
}

struct temp_t get_temperature_reading()
{
    struct temp_t result;
    //todo
    return result;
}

unsigned int get_humidity_reading()
{
    //todo
    return 0;
}

void set_relay_state(bool closed)
{
    //todo
}