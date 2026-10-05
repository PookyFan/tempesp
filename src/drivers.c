/* Driver for HDC1080 - I2C temperature and humidity sensor */

#include "espressif/esp_common.h"

#include "i2c/i2c.h"
#include "utils.h"
#include "drivers.h"

#define BUS     0
#define SCL_PIN 5
#define SDA_PIN 4

#define SENSOR_I2C_ADDR     0x40

#define TEMP_READING_REG    0x00
#define CONFIGURATION_REG   0x02
#define MANUFACTURER_ID_REG 0xFE
#define DEVICE_ID_REG       0xFF

//Hardware IDs have their bytes swapped, as they are received
//on I2c with MSB first, while ESP8266's MCU is little-endian
#define MANUFACTURER_ID_VAL 0x4954
#define DEVICE_ID_VAL       0x5010 

#define CONFIG_VAL          0x10
#define RESET_VAL           0x80

bool initialize_sensor()
{
    uint8_t m_id[2] ALIGNAS(uint16_t) = {MANUFACTURER_ID_REG};
    uint8_t d_id[2] ALIGNAS(uint16_t) = {DEVICE_ID_REG};
    if(i2c_init(BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K))
    {
        printf("Failed to initialize I2C bus\n");
        return false;
    }

    if( i2c_slave_read(BUS, SENSOR_I2C_ADDR, m_id, m_id, ARRAY_SIZE(m_id))
     || i2c_slave_read(BUS, SENSOR_I2C_ADDR, d_id, d_id, ARRAY_SIZE(d_id)))
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

    uint8_t reg = CONFIGURATION_REG;
    uint8_t conf[2] = {RESET_VAL, 0x0};
    if(i2c_slave_write(BUS, SENSOR_I2C_ADDR, &reg, conf, ARRAY_SIZE(conf)))
    {
        printf("Failed to reset sensor\n");
        return false;
    }

    vTaskDelay(20 / portTICK_PERIOD_MS); //Datasheet says sensor needs 15 ms to be ready
    return true;
}

bool get_measurements(struct measurements_t* result)
{
    uint8_t reg = CONFIGURATION_REG;
    uint8_t conf[2] = {CONFIG_VAL, 0x0};
    if(i2c_slave_write(BUS, SENSOR_I2C_ADDR, &reg, conf, ARRAY_SIZE(conf)))
    {
        printf("Failed to configure sensor\n");
        return false;
    }

    reg = TEMP_READING_REG;
    if(i2c_slave_write(BUS, SENSOR_I2C_ADDR, NULL, &reg, 1)) //just write register address
    {
        printf("Failed to start measurement\n");
        return false;
    }

    int retries = 10;
    uint8_t data[4];
    do
    {
        vTaskDelay(10 / portTICK_PERIOD_MS);
        if(i2c_slave_read(BUS, SENSOR_I2C_ADDR, NULL, data, sizeof(data)))
            printf("Measurements unavailable yet...\n");
        else break;

    } while(--retries);

    if(!retries)
    {
        printf("Measurements reading timed out!\n");
        return false;
    }

    printf("Data read from sensor: 0x%x 0x%x 0x%x 0x%x\n", data[0], data[1], data[2], data[3]);

    //As per HDC1080 datasheet, temperature is [(reading / 2^16) * 165 - 40] *C,
    //but we get only 14 of 16 bits, since two LSB are not part of actual reading.
    //We want to get integer and decimal parts separately, so we'll need to be
    //a little creative with calculation of temperature value parts
    unsigned int reading = (data[0] << 6) | (data[1] >> 2); //Shift right by 2 now...
    reading *= 16500;
    reading >>= 14; //...so now we only need to shift right by 14

    int integer_part = reading / 100;
    result->temperature.integer_part = integer_part - 40;
    result->temperature.fractional_part = reading - (integer_part * 100);
    printf("Temperature read: %d.%02d *C\n", result->temperature.integer_part, result->temperature.fractional_part);

    //As per HDC1080 datasheet, humidity is (reading / 2^16) * 100%, but again
    //only 14 MSB are used, so we'll do similar math as above
    reading = (data[2] << 6) | (data[3] >> 2);
    reading *= 10000;
    reading >>= 14;
    integer_part = reading / 100;
    result->humidity.integer_part = integer_part;
    result->humidity.fractional_part = reading - (integer_part * 100);
    printf("Humidity read: %d.%02d%%\n", result->humidity.integer_part, result->humidity.fractional_part);

    return true;
}

void set_relay_state(bool closed)
{
    //todo
}