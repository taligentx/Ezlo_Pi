#include "ezlopi_devices_list.h"
#include "ezlopi_device_value_updated.h"
#include "ezlopi_cloud_category_str.h"
#include "ezlopi_cloud_subcategory_str.h"
#include "ezlopi_item_name_str.h"
#include "ezlopi_cloud_device_types_str.h"
#include "ezlopi_cloud_value_type_str.h"
#include "ezlopi_adc.h"

#include "esp_err.h"
#include "items.h"
#include "trace.h"
#include "cJSON.h"

#include "027_sens_water_leak_sensor.h"

static const char *ezlopi_water_present_leak_state = NULL;

static int water_leak_sensor_prepare_and_add(void *args);
static s_ezlopi_device_properties_t *water_leak_sensor_prepare(cJSON *cjson_device);
static int water_leak_sensor_init(s_ezlopi_device_properties_t *properties);
static int get_water_leak_sensor_value_to_cloud(s_ezlopi_device_properties_t *properties, void *args);
static int water_leak_sensor_get_value(s_ezlopi_device_properties_t *properties);

int water_leak_sensor(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *ezlo_device, void *arg, void *user_arg)
{
    int ret = 0;

    switch (action)
    {
    case EZLOPI_ACTION_PREPARE:
    {
        ret = water_leak_sensor_prepare_and_add(arg);
        break;
    }
    case EZLOPI_ACTION_INITIALIZE:
    {
        ret = water_leak_sensor_init(ezlo_device);
        break;
    }
    case EZLOPI_ACTION_GET_EZLOPI_VALUE:
    {
        get_water_leak_sensor_value_to_cloud(ezlo_device, arg);
        break;
    }
    case EZLOPI_ACTION_NOTIFY_200_MS:
    {
        water_leak_sensor_get_value(ezlo_device);
        break;
    }
    default:
    {
        break;
    }
    }
    return ret;
}

static int water_leak_sensor_prepare_and_add(void *args)
{
    int ret = 0;
    s_ezlopi_prep_arg_t *device_prep_arg = (s_ezlopi_prep_arg_t *)args;

    if ((NULL != device_prep_arg) && (NULL != device_prep_arg->cjson_device))
    {
        s_ezlopi_device_properties_t *water_leak_sensor_properties = water_leak_sensor_prepare(device_prep_arg->cjson_device);
        if (water_leak_sensor_properties)
        {
            if (0 == ezlopi_devices_list_add(device_prep_arg->device, water_leak_sensor_properties, NULL))
            {
                free(water_leak_sensor_properties);
            }
            else
            {
                ret = 1;
            }
        }
    }

    return ret;
}

static s_ezlopi_device_properties_t *water_leak_sensor_prepare(cJSON *cjson_device)
{
    s_ezlopi_device_properties_t *water_leak_sensor_properties = malloc(sizeof(s_ezlopi_device_properties_t));

    if (water_leak_sensor_properties)
    {
        memset(water_leak_sensor_properties, 0, sizeof(s_ezlopi_device_properties_t));
        water_leak_sensor_properties->interface_type = EZLOPI_DEVICE_INTERFACE_ANALOG_INPUT;

        char *device_name = NULL;
        CJSON_GET_VALUE_STRING(cjson_device, "dev_name", device_name);
        ASSIGN_DEVICE_NAME(water_leak_sensor_properties, device_name);
        water_leak_sensor_properties->ezlopi_cloud.category = category_security_sensor;
        water_leak_sensor_properties->ezlopi_cloud.subcategory = subcategory_leak;
        water_leak_sensor_properties->ezlopi_cloud.item_name = ezlopi_item_name_water_leak_alarm;
        water_leak_sensor_properties->ezlopi_cloud.device_type = dev_type_sensor;
        water_leak_sensor_properties->ezlopi_cloud.value_type = value_type_token;
        water_leak_sensor_properties->ezlopi_cloud.has_getter = true;
        water_leak_sensor_properties->ezlopi_cloud.has_setter = false;
        water_leak_sensor_properties->ezlopi_cloud.reachable = true;
        water_leak_sensor_properties->ezlopi_cloud.battery_powered = false;
        water_leak_sensor_properties->ezlopi_cloud.show = true;
        water_leak_sensor_properties->ezlopi_cloud.room_name[0] = '\0';
        water_leak_sensor_properties->ezlopi_cloud.device_id = ezlopi_device_generate_device_id();
        water_leak_sensor_properties->ezlopi_cloud.room_id = ezlopi_device_generate_room_id();
        water_leak_sensor_properties->ezlopi_cloud.item_id = ezlopi_device_generate_item_id();

        CJSON_GET_VALUE_INT(cjson_device, "gpio", water_leak_sensor_properties->interface.adc.gpio_num);
        water_leak_sensor_properties->interface.adc.resln_bit = 3;
        water_leak_sensor_properties->user_arg
    }

    return water_leak_sensor_properties;
}

static int water_leak_sensor_init(s_ezlopi_device_properties_t *properties)
{
    int ret = 0;
    if (GPIO_IS_VALID_GPIO(properties->interface.adc.gpio_num))
    {
        ezlopi_adc_init(properties->interface.adc.gpio_num, properties->interface.adc.resln_bit);
        ret = 1;
    }
    return ret;
}

static int get_water_leak_sensor_value_to_cloud(s_ezlopi_device_properties_t *properties, void *arg)
{
    int ret = 0;
    cJSON *cjson_propertise = (cJSON *)arg;
    if (cjson_propertise)
    {
        cJSON_AddStringToObject(cjson_propertise, "value", ezlopi_water_present_leak_state);
        ret = 1;
    }
    return ret;
}

static int water_leak_sensor_get_value(s_ezlopi_device_properties_t *properties)
{
    int ret = 0;
    static char *ezlopi_water_previous_leak_state;
    const static char *_no_water_leak = "no_water_leak";
    const static char *_water_leak_detected = "water_leak_detected";
    s_ezlopi_analog_data_t ezlopi_analog_data = {.value = 0, .voltage = 0};

    ezlopi_adc_get_adc_data(properties->interface.adc.gpio_num, &ezlopi_analog_data);
    TRACE_B("Value is: %d, voltage is: %d", ezlopi_analog_data.value, ezlopi_analog_data.value);

    if (1000 <= ezlopi_analog_data.voltage)
    {
        ezlopi_water_present_leak_state = _water_leak_detected;
    }
    else
    {
        ezlopi_water_present_leak_state = _no_water_leak;
    }

    if (ezlopi_water_previous_leak_state != ezlopi_water_present_leak_state)
    {
        ezlopi_device_value_updated_from_device(properties);
        ezlopi_water_previous_leak_state = ezlopi_water_present_leak_state;
    }

    return ret;
}
