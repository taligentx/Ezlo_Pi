#ifndef __EZLOPI_DEVICES_LIST_H__
#define __EZLOPI_DEVICES_LIST_H__

#include <stdint.h>
#include <stdbool.h>
#include <cJSON.h>
#include "ezlopi_actions.h"
#include "ezlopi_devices.h"
#include "sdkconfig.h"

// This Preprocessor must be defined to select a device
#define EZLOPI_DEVICE_TYPE  EZLOPI_DEVICE_TYPE_GENERIC

#define EZLOPI_SENSOR_NONE                                              0

#define EZLOPI_DEVICE_0001_DIGITAL_OUT_LED                              1
#define EZLOPI_DEVICE_0002_DIGITAL_OUT_RELAY                            2
#define EZLOPI_DEVICE_0003_DIGITAL_OUT_PLUG                             3
#define EZLOPI_SENSOR_0004_DIGITAL_IN_SWITCH                            4

#define EZLOPI_SENSOR_0005_I2C_MPU6050                                  5
#define EZLOLPI_SENSOR_0006_I2C_ADXL345                                 6
#define EZLOLPI_SENSOR_0007_I2C_GY271                                   7
#define EZLOLPI_SENSOR_0008_I2C_LTR303AL                                8
#define EZLOLPI_DEVICE_0009_OTHER_RMT_SK6812                            9
#define EZLOLPI_SENSOR_0010_I2C_BME680                                  10
#define EZLOLPI_SENSOR_0011_I2C_MAX30100                                11
#define EZLOPI_SENSOR_0012_I2C_BME280                                   12
#define EZLOPI_SENSOR_0013_SPI_BME280                                   13
#define EZLOPI_SENSOR_0014_UART_PMS5003                                 14
#define EZLOPI_SENSOR_0015_ONE_WIRE_DHT11                               15
#define EZLOPI_SENSOR_0016_ONE_WIRE_DHT22                               16
#define EZLOPI_SENSOR_0017_ADC_POTENTIOMETER                            17

#if CONFIG_IDF_TARGET_ESP32
#define EZLOPI_SENSOR_0018_OTHER_INTERNAL_HALL_EFFECT                   18
#endif

#define EZLOPI_SENSOR_0019_DIGITAL_INPUT_PIR                            19
#define EZLOPI_SENSOR_0020_ADC_JOYSTICK_2_AXIS                          20
#define EZLOPI_SENSOR_0021_UART_MB1013                                  21
#define EZLOPI_DEVICE_0022_PWM_DIMMABLE_BULB                            22
#define EZLOPI_SENSOR_0023_DIGITAL_IN_TTP223B_TOUCH_SWITCH              23

#if CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3
#define EZLOPI_SENSOR_0024_OTHER_HCSR04                                 24
#endif

#define EZLOPI_SENSOR_0025_DIGITAL_IN_LDR_DIGITAL                       25
#define EZLOPI_SENSOR_0026_ADC_LDR                                      26
#define EZLOPI_SENSOR_0027_ADC_WATER_LEAK                               27
#define EZLOPI_SENSOR_0028_ADC_GY61                                     28
#define EZLOPI_SENSOR_0029_I2C_GXHTC3                                   29
#define EZLOPI_SENSOR_0030_ONE_WIRE_DS18B20                             30
#define EZLOPI_SENSOR_0031_OTHER_JSNSR04T                               31
#define EZLOPI_SENSOR_0032_ADC_SOIL_MOISTURE                            32
#define EZLOPI_SENSOR_0033_ADC_TURBIDITY                                33
#define EZLOPI_SENSOR_0034_DIGITAL_IN_PROXIMITY                         34
#define EZLOPI_SENSOR_0035_DIGITAL_IN_TPP223B_TOUCH_SENSOR              35

/**
 * @brief defining the type of sensor call function
 *
 */
// int sensor_bme280(e_ezlopi_actions_t action, void *arg);
typedef int (*f_sensor_call_t)(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg);
typedef struct s_ezlopi_device
{
    uint32_t id;
    f_sensor_call_t func;
} s_ezlopi_device_t;

typedef struct l_ezlopi_configured_devices
{
    void *user_arg;
    s_ezlopi_device_t *device;
    s_ezlopi_device_properties_t *properties;
    struct l_ezlopi_configured_devices *next;

} l_ezlopi_configured_devices_t;

typedef struct s_ezlopi_prep_arg
{
    cJSON *cjson_device;
    s_ezlopi_device_t *device;

} s_ezlopi_prep_arg_t;

/**
 * @brief Provides the list of available sensors
 *
 * @return f_sensor_call_t*
 */
s_ezlopi_device_t *ezlopi_devices_list_get_list(void);

// void ezlopi_devices_list_add(s_ezlopi_device_t *device);
// int ezlopi_devices_list_add(s_ezlopi_device_t *device);
// int ezlopi_devices_list_add(s_ezlopi_device_t *device, s_ezlopi_device_properties_t *properties);
int ezlopi_devices_list_add(s_ezlopi_device_t *device, s_ezlopi_device_properties_t *properties, void *user_arg);
l_ezlopi_configured_devices_t *ezlopi_devices_list_get_configured_items(void);

#if 0
/**
 * @brief 'ezlopi_sensor_get_next'
 *
 * @param current_sensor
 * @return s_sensors_list_t*
 */
s_sensors_list_t *ezlopi_sensor_get_next(s_sensors_list_t *current_sensor);
#endif

#endif // __EZLOPI_DEVICES_LIST_H__
