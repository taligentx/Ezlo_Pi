#ifndef __EZLOPI_DEVICES_LIST_H__
#define __EZLOPI_DEVICES_LIST_H__

#include <stdint.h>
#include <stdbool.h>
#include "ezlopi_actions.h"
#include "ezlopi_devices.h"

#define EZLOPI_SENSOR_NONE 0
#define EZLOPI_SENSOR_0001_LED 1
#define EZLOPI_SENSOR_0002_RELAY 2
#define EZLOPI_SENSOR_0003_PLUG 3
#define EZLOPI_SENSOR_0018_BME280 18

/**
 * @brief defining the type of sensor call function
 *
 */
// int sensor_bme280(e_ezlopi_actions_t action, void *arg);
typedef int (*f_sensor_call_t)(e_ezlopi_actions_t action, void *arg);
typedef struct s_ezlopi_device
{
    uint16_t id;
    f_sensor_call_t func;
    // bool is_configured;
    s_ezlopi_device_properties_t *properties;
} s_ezlopi_device_t;

typedef struct l_ezlopi_configured_devices
{
    struct l_ezlopi_configured_devices *next;
    s_ezlopi_device_t *device;
} l_ezlopi_configured_devices_t;

/**
 * @brief Provides the list of available sensors
 *
 * @return f_sensor_call_t*
 */
s_ezlopi_device_t *ezlopi_devices_list_get_list(void);

void ezlopi_devices_list_add(s_ezlopi_device_t *device);
l_ezlopi_configured_devices_t *ezlopi_sensor_get_configured_items(void);

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