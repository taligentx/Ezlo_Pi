#include "ezlopi_actions.h"
#include "ezlopi_devices_list.h"
#include "ezlopi_factory_info.h"
#include "device_0001_digitalOut_generic.h"
#include "device_0002_digitalOut_relay.h"
#include "device_0003_digitalOut_plug.h"
#include "sensor_0019_digitalIn_PIR.h"
#include "sensor_0018_internal_hall_effect.h"
#include "device_0022_PWM_dimmable_lamp.h"
#include "sensor_0020_ADC_2axis_joystick.h"
#include "sensor_0021_UART_MB1013.h"
#include "device_0036_PWM_servo_MG996R.h"
#include "sensor_0024_other_HCSR04.h"
#include "sensor_0023_digitalIn_touch_switch_TTP223B.h"
#include "sensor_0025_digitalIn_LDR.h"
#include "sensor_0026_ADC_LDR.h"
#include "sensor_0027_ADC_waterLeak.h"
#include "sensor_0029_I2C_GXHTC3.h"
#include "sensor_0030_oneWire_DS18B20.h"
#include "sensor_0035_digitalIn_touch_sensor_TPP223B.h"
#include "sensor_0016_OneWire_DHT22.h"
#include "sensor_0032_ADC_soilMoisture.h"
#include "sensor_0031_other_JSNSR04T.h"
#include "sensor_0017_ADC_potentiometer.h"
#include "sensor_0005_I2C_MPU6050.h"
#include "sensor_0006_I2C_ADXL345.h"

static s_ezlopi_device_t device_array[] = {

#if (EZLOPI_SWITCH_BOX == EZLOPI_DEVICE_TYPE)
#ifdef EZLOPI_SENSOR_0001_LED
    {
        .id = EZLOPI_SENSOR_0001_LED,
        .func = device_0001_digitalOut_generic,
    },
#endif
#ifdef EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C
    {
        .id = EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C,
        .func = gxhtc3_rh_t_sensor,
    },
#endif
#else

#ifdef EZLOPI_SENSOR_0001_LED
    {
        .id = EZLOPI_SENSOR_0001_LED,
        .func = device_0001_digitalOut_generic,
    },
#endif

#ifdef EZLOPI_SENSOR_0002_RELAY
    {
        .id = EZLOPI_SENSOR_0002_RELAY,
        .func = device_0002_digitalOut_relay,
    },
#endif

#ifdef EZLOPI_SENSOR_0003_PLUG
    {
        .id = EZLOPI_SENSOR_0003_PLUG,
        .func = device_0003_digitalOut_plug,
    },
#endif

#ifdef EZLOLPI_SENSOR_006_ADXL345_ACCELEROMETER
    {
        .id = EZLOLPI_SENSOR_006_ADXL345_ACCELEROMETER,
        .func = sensor_i2c_ADXL345_accelerometer
    },
#endif

// #ifdef EZLOPI_SENSOR_0012_BME280_I2C
//     {
//         .id = EZLOPI_SENSOR_0012_BME280_I2C,
//         .func = sensor_bme280,
//     },
// #endif
#ifdef EZLOPI_SENSOR_0016_DHT22_SENSOR
    {
        .id = EZLOPI_SENSOR_0016_DHT22_SENSOR,
        .func = dht22_sensor,
    },
#endif

#ifdef EZLOPI_SENSOR_0017_POTENTIOMETER
    {
        .id = EZLOPI_SENSOR_0017_POTENTIOMETER,
        .func = potentiometer,
    },
#endif

#ifdef EZLOPI_SENSOR_0018_DOOR
    {
        .id = EZLOPI_SENSOR_0018_DOOR,
        .func = door_hall_sensor,
    },
#endif

#ifdef EZLOPI_SENSOR_0019_PIR
    {
        .id = EZLOPI_SENSOR_0019_PIR,
        .func = sensor_0019_digitalIn_PIR,
    },
#endif

#ifdef EZLOPI_SENSOR_0020_JOYSTICK_2_AXIS
    {
        .id = EZLOPI_SENSOR_0020_JOYSTICK_2_AXIS,
        .func = sensor_0020_ADC_2axis_joystick,
    },
#endif

#ifdef EZLOPI_SENSOR_0021_ULTRASONIC_HRLV_MAXSENSOR_EZ_MB1013
    {
        .id = EZLOPI_SENSOR_0021_ULTRASONIC_HRLV_MAXSENSOR_EZ_MB1013,
        .func = sensor_0021_UART_MB1013,
    },
#endif

#ifdef EZLOPI_SENSOR_0022_DIMMABLE_BULB
    {
        .id = EZLOPI_SENSOR_0022_DIMMABLE_BULB,
        .func =  device_0022_PWM_dimmable_lamp, // ezlopi_servo_motor_MG_996R,
    },
#endif

#ifdef EZLOPI_SENSOR_0023_TTP_223B_TOUCH_SENSOR_SWITCH
    {
        .id = EZLOPI_SENSOR_0023_TTP_223B_TOUCH_SENSOR_SWITCH,
        .func = sensor_0023_digitalIn_touch_switch_TTP223B,
    },
#endif

#ifdef EZLOPI_SENSOR_0024_ULTRASONIC_HC_SR04_SENSOR
    {
        .id = EZLOPI_SENSOR_0024_ULTRASONIC_HC_SR04_SENSOR,
        .func = sensor_0024_other_HCSR04,
    },
#endif

#ifdef EZLOPI_SENSOR_0025_LDR_DIGITAL_MODULE_SENSOR
    {
        .id = EZLOPI_SENSOR_0025_LDR_DIGITAL_MODULE_SENSOR,
        .func = sensor_0025_digitalIn_LDR,
    },
#endif

#ifdef EZLOPI_SENSOR_0026_LDR_ANALOG_SENSOR
    {
        .id = EZLOPI_SENSOR_0026_LDR_ANALOG_SENSOR,
        .func = sensor_0026_ADC_LDR,
    },
#endif

#ifdef EZLOPI_SENSOR_0027_WATER_LEAK
    {
        .id = EZLOPI_SENSOR_0027_WATER_LEAK,
        .func = sensor_0027_ADC_waterLeak,
    },
#endif

// #ifdef EZLOPI_SENSOR_0028_SOUND_SENSOR_SPI
//     {
//         .id = EZLOPI_SENSOR_0028_SOUND_SENSOR_SPI,
//         .func = sound_sensor,
//     },
// #endif
#ifdef EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C
    {
        .id = EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C,
        .func = gxhtc3_rh_t_sensor,
    },
#endif

#ifdef EZLOPI_SENSOR_030_DS18B20
    {
        .id = EZLOPI_SENSOR_030_DS18B20,
        .func = sensor_0030_oneWire_DS18B20,
    },
#endif

#ifdef EZLOPI_SENSOR_031_JSN_SR04T_WATER_LEVEL_SENSOR
    {
        .id = EZLOPI_SENSOR_031_JSN_SR04T_WATER_LEVEL_SENSOR,
        .func = sensor_0031_other_JSNSR04T,
    },
#endif

#ifdef EZLOPI_SENSOR_032_SOIL_MOISTURE
    {
        .id = EZLOPI_SENSOR_032_SOIL_MOISTURE,
        .func = sensor_0032_ADC_soilMoisture,
    },
#endif

#ifdef EZLOPI_SENSOR_1024_DEVICE_HEALTH
    {
        .id = EZLOPI_SENSOR_1024_DEVICE_HEALTH,
        .func = device_health,
    },
#endif

#ifdef EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C
    {
        .id = EZLOPI_SENSOR_0029_GXHTC3_RH_T_I2C,
        .func = gxhtc3_rh_t_sensor,
    },
#endif
#ifdef EZLOPI_SENSOR_0035_TPP_223B_TOUCH_SENSOR
    {
        .id = EZLOPI_SENSOR_0035_TPP_223B_TOUCH_SENSOR,
        .func = sensor_0035_digitalIn_touch_sensor_TPP223B,
    },
#endif
#ifdef EZLOPI_SENSOR_0005_MPU6050_ACCEL_GYRO_TEMP
{
        .id = EZLOPI_SENSOR_0005_MPU6050_ACCEL_GYRO_TEMP,
        .func = sensor_0005_I2C_MPU6050,
},
#endif
#endif
    /**
     * @brief 'EZLOPI_SENSOR_NONE' must not be removed from this array.
     * This is essential for terminating the loop termination of loop.
     */
    {
        .id = EZLOPI_SENSOR_NONE,
        .func = NULL,
    },

};

s_ezlopi_device_t *ezlopi_devices_list_get_list(void)
{
    return device_array;
}

static l_ezlopi_configured_devices_t *configured_device = NULL;
static l_ezlopi_configured_devices_t *ezlopi_device_list_create(s_ezlopi_device_t *device, s_ezlopi_device_properties_t *properties, void *user_arg);

l_ezlopi_configured_devices_t *ezlopi_devices_list_get_configured_items(void)
{
    return configured_device;
}

int ezlopi_devices_list_add(s_ezlopi_device_t *device, s_ezlopi_device_properties_t *properties, void *user_arg)
{
    int ret = 0;
    if (configured_device)
    {
        l_ezlopi_configured_devices_t *current_dev = configured_device;

        while (NULL != current_dev->next)
        {
            current_dev = current_dev->next;
        }

        current_dev->next = ezlopi_device_list_create(device, properties, user_arg);
        if (current_dev->next)
        {
            ret = 1;
        }
    }
    else
    {
        configured_device = ezlopi_device_list_create(device, properties, user_arg);
        if (configured_device)
        {
            ret = 1;
        }
    }

    return ret;
}

static l_ezlopi_configured_devices_t *ezlopi_device_list_create(s_ezlopi_device_t *device, s_ezlopi_device_properties_t *properties, void *user_arg)
{
    l_ezlopi_configured_devices_t *device_list_element = (l_ezlopi_configured_devices_t *)malloc(sizeof(l_ezlopi_configured_devices_t));
    if (device_list_element)
    {
        memset(device_list_element, 0, sizeof(l_ezlopi_configured_devices_t));
        device_list_element->device = device;
        device_list_element->properties = properties;
        device_list_element->next = NULL;
        if (user_arg)
        {
            device_list_element->user_arg = user_arg;
        }
    }
    return device_list_element;
}
