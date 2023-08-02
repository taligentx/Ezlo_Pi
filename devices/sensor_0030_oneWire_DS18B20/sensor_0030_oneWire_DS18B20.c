

#include "ezlopi_cloud.h"
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

#include "sensor_0030_oneWire_DS18B20.h"
#include "ds18b20_onewire.h"


static int sensor_0030_oneWire_DS18B20_prepare_and_add(void *args);
static s_ezlopi_device_properties_t *sensor_0030_oneWire_DS18B20_prepare(cJSON *cjson_device);
static int sensor_0030_oneWire_DS18B20_init(s_ezlopi_device_properties_t *properties);
static int get_sensor_0030_oneWire_DS18B20_value_to_cloud(s_ezlopi_device_properties_t *properties, void *args);

    
static esp_err_t ds18b20_write_data(uint8_t* data, uint32_t gpio_pin);
static esp_err_t ds18b20_read_data(uint8_t* data, uint32_t gpio_pin);
static bool ds18b20_reset_line(uint32_t gpio_pin);
static esp_err_t ds18b20_write_to_scratchpad(uint8_t th_val, uint8_t tl_val, uint8_t resolution, uint8_t gpio_pin);
static bool ds18b20_recognize_device(uint32_t gpio_pin);
static esp_err_t ds18b20_get_temperature_data(double* temperature_data, uint32_t gpio_pin);
static uint8_t ds18b20_calculate_crc(const uint8_t* data, uint8_t len);


int sensor_0030_oneWire_DS18B20(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *ezlo_device, void *arg, void *user_arg)
{
    int ret = 0;

    switch (action)
    {
        case EZLOPI_ACTION_PREPARE:
        {
            ret = sensor_0030_oneWire_DS18B20_prepare_and_add(arg);
            break;
        }
        case EZLOPI_ACTION_INITIALIZE:
        {
            ret = sensor_0030_oneWire_DS18B20_init(ezlo_device);
            break;
        }
        case EZLOPI_ACTION_GET_EZLOPI_VALUE:
        {
            ret = get_sensor_0030_oneWire_DS18B20_value_to_cloud(ezlo_device, arg);
            break;
        }
        case EZLOPI_ACTION_NOTIFY_1000_MS:
        {
            ezlopi_device_value_updated_from_device(ezlo_device);
            break;
        }
        default:
        {
            break;
        }
    }
    return ret;
}


static int sensor_0030_oneWire_DS18B20_prepare_and_add(void *args)
{
    int ret = 0;
    s_ezlopi_prep_arg_t *device_prep_arg = (s_ezlopi_prep_arg_t *)args;

    if ((NULL != device_prep_arg) && (NULL != device_prep_arg->cjson_device))
    {
        s_ezlopi_device_properties_t *sensor_0030_oneWire_DS18B20_properties = sensor_0030_oneWire_DS18B20_prepare(device_prep_arg->cjson_device);
        if (sensor_0030_oneWire_DS18B20_properties)
        {
            if (0 == ezlopi_devices_list_add(device_prep_arg->device, sensor_0030_oneWire_DS18B20_properties, NULL))
            {
                free(sensor_0030_oneWire_DS18B20_properties);
            }
            else
            {
                ret = 1;
            }
        }
    }

    return ret;
}


static s_ezlopi_device_properties_t *sensor_0030_oneWire_DS18B20_prepare(cJSON *cjson_device)
{
    s_ezlopi_device_properties_t *sensor_0030_oneWire_DS18B20_properties = malloc(sizeof(s_ezlopi_device_properties_t));

    if (sensor_0030_oneWire_DS18B20_properties)
    {
        memset(sensor_0030_oneWire_DS18B20_properties, 0, sizeof(s_ezlopi_device_properties_t));
        sensor_0030_oneWire_DS18B20_properties->interface_type = EZLOPI_DEVICE_INTERFACE_ANALOG_INPUT;

        char *device_name = NULL;
        CJSON_GET_VALUE_STRING(cjson_device, "dev_name", device_name);
        ASSIGN_DEVICE_NAME(sensor_0030_oneWire_DS18B20_properties, device_name);
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.category = category_temperature;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.subcategory = subcategory_not_defined;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.item_name = ezlopi_item_name_temp;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.device_type = dev_type_sensor;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.value_type = value_type_temperature;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.has_getter = true;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.has_setter = false;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.reachable = true;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.battery_powered = false;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.show = true;
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.room_name[0] = '\0';
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.device_id = ezlopi_cloud_generate_device_id();
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.room_id = ezlopi_cloud_generate_room_id();
        sensor_0030_oneWire_DS18B20_properties->ezlopi_cloud.item_id = ezlopi_cloud_generate_item_id();

        CJSON_GET_VALUE_INT(cjson_device, "gpio", sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.gpio_num);
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_out.enable = false;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.enable = true;
        // sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.gpio_num = 2;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.interrupt = GPIO_INTR_DISABLE;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.invert = false;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.mode = GPIO_MODE_DISABLE;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.pull = GPIO_FLOATING;
        sensor_0030_oneWire_DS18B20_properties->interface.gpio.gpio_in.value = 0;
        
    }
    return sensor_0030_oneWire_DS18B20_properties;
}

static int sensor_0030_oneWire_DS18B20_init(s_ezlopi_device_properties_t *properties)
{
    int ret = 0;
    if(ds18b20_reset_line(properties->interface.gpio.gpio_in.gpio_num))
    {
        if(ds18b20_recognize_device(properties->interface.gpio.gpio_in.gpio_num))
        {
            TRACE_B("Providing initial settings to DS18B20.");
            ds18b20_write_to_scratchpad(DS18B20_TH_HIGHER_THRESHOLD, DS18B20_TL_LOWER_THRESHOLD, 12, properties->interface.gpio.gpio_in.gpio_num);
            ret = 1;
        }
    }
    return ret;
}

static int get_sensor_0030_oneWire_DS18B20_value_to_cloud(s_ezlopi_device_properties_t *properties, void *args)
{
    int ret = 0;
    double temperature = 0;
    cJSON* cjson_properties = (cJSON*)args;
    
    if(cjson_properties)
    {
        ds18b20_get_temperature_data(&temperature, properties->interface.gpio.gpio_in.gpio_num);
        TRACE_B("Temperature is: %f degree censius", temperature);
        cJSON_AddNumberToObject(cjson_properties, "value", temperature);
        cJSON_AddStringToObject(cjson_properties, "scale", "celsius");
    }

    return ret;
}

static esp_err_t ds18b20_write_data(uint8_t* data, uint32_t gpio_pin)
{
    esp_err_t error = ESP_OK;
    error = one_wire_write_byte_to_line(data, gpio_pin);
    return error;
}

static esp_err_t ds18b20_read_data(uint8_t* data, uint32_t gpio_pin)
{
    esp_err_t error = ESP_OK;
    error = one_wire_read_byte_from_line(data, gpio_pin);
    return error;
}

static esp_err_t ds18b20_write_to_scratchpad(uint8_t th_val, uint8_t tl_val, uint8_t resolution, uint8_t gpio_pin)
{
    esp_err_t error = ESP_OK;
    uint8_t ds18b20_skip_rom = DS18B20_ROM_COMMAND_SKIP_ROM;
    uint8_t ds18b20_write_scratchpad = DS18B20_FUNCTION_COMMAND_WRITE_SCRATCHPAD;
    uint8_t ds18b20_th_val = th_val;
    uint8_t ds18b20_tl_vla = tl_val;
    uint8_t ds18b20_resolution = 0;

    switch (resolution)
    {
        case 9:
        {
            ds18b20_resolution = DS18B20_TEMPERATURE_9_BIT_RESOLUTION;
            break;
        }
        case 10:
        {
            ds18b20_resolution = DS18B20_TEMPERATURE_10_BIT_RESOLUTION;
            break;
        }
        case 11:
        {
            ds18b20_resolution = DS18B20_TEMPERATURE_11_BIT_RESOLUTION;
            break;
        }
        case 12:
        default:
        {
            ds18b20_resolution = DS18B20_TEMPERATURE_12_BIT_RESOLUTION;
            break;
        }
    }

    if(ds18b20_reset_line(gpio_pin))
    {
        ds18b20_write_data(&ds18b20_skip_rom, gpio_pin);
        // write to the scratch pad. (TH, TL, Configuration) = (42, -10, 12-bit resolution.)
        ds18b20_write_data(&ds18b20_write_scratchpad, gpio_pin);
        ds18b20_write_data(&ds18b20_th_val, gpio_pin);
        ds18b20_write_data(&ds18b20_tl_vla, gpio_pin);
        ds18b20_write_data(&ds18b20_resolution, gpio_pin);
    }
    else 
    {
        error = ESP_FAIL;
    }
    return error;
}

static bool ds18b20_reset_line(uint32_t gpio_pin)
{
    bool present = false;
    present = one_wire_reset_line(gpio_pin);
    return present;
}

static bool ds18b20_recognize_device(uint32_t gpio_pin)
{
    uint8_t read_rom_command = 0x33;
    ds18b20_write_data(&read_rom_command, gpio_pin);
    uint8_t data_from_ds18b20 = 0;
    ds18b20_read_data(&data_from_ds18b20, gpio_pin);
    return (data_from_ds18b20 == DS18B20_FAMILY_CODE) ? true : false;
}

static esp_err_t ds18b20_get_temperature_data(double* temperature_data, uint32_t gpio_pin)
{
    esp_err_t error = ESP_OK;
    uint8_t ds18b20_skip_rom = DS18B20_ROM_COMMAND_SKIP_ROM;
    uint8_t ds18b20_convert_temperature = DS18B20_FUNCTION_COMMAND_CONVERT_TEMP;
    uint8_t ds18b20_read_scratchpad = DS18B20_FUNCTION_COMMAND_READ_SCRATCHPAD;
    
    uint8_t ds18b20_temperature_lsb = 0;
    uint8_t ds18b20_temperature_msb = 0;
    uint8_t ds18b20_th_value = 0;
    uint8_t ds18b20_tl_data = 0;
    uint8_t ds18b20_config_data = 0;
    uint8_t ds18b20_reserved_reg_1 = 0;
    uint8_t ds18b20_reserved_reg_2 = 0;
    uint8_t ds18b20_reserved_reg_3 = 0;
    uint8_t ds18b20_crc = 0;

    if(ds18b20_reset_line(gpio_pin))
    {
        ds18b20_write_data(&ds18b20_skip_rom, gpio_pin);
        ds18b20_write_data(&ds18b20_convert_temperature, gpio_pin);
        vTaskDelay(750 / portTICK_RATE_MS);
        ds18b20_reset_line(gpio_pin);
        ds18b20_write_data(&ds18b20_skip_rom, gpio_pin);
        ds18b20_write_data(&ds18b20_read_scratchpad, gpio_pin);

        // Now read the data;
        ds18b20_read_data(&ds18b20_temperature_lsb, gpio_pin);
        ds18b20_read_data(&ds18b20_temperature_msb, gpio_pin);
        ds18b20_read_data(&ds18b20_th_value, gpio_pin);
        ds18b20_read_data(&ds18b20_tl_data, gpio_pin);
        ds18b20_read_data(&ds18b20_config_data, gpio_pin);
        ds18b20_read_data(&ds18b20_reserved_reg_1, gpio_pin);
        ds18b20_read_data(&ds18b20_reserved_reg_2, gpio_pin);
        ds18b20_read_data(&ds18b20_reserved_reg_3, gpio_pin);
        ds18b20_read_data(&ds18b20_crc, gpio_pin);

        uint8_t ds18b20_scratchpad_data_array[8] = {
            ds18b20_temperature_lsb,
            ds18b20_temperature_msb,
            ds18b20_th_value,
            ds18b20_tl_data,
            ds18b20_config_data,
            ds18b20_reserved_reg_1,
            ds18b20_reserved_reg_2,
            ds18b20_reserved_reg_3,
        };

        uint8_t calculated_crc = ds18b20_calculate_crc(ds18b20_scratchpad_data_array, 8);
        if(calculated_crc != ds18b20_crc)
        {
            TRACE_B("CRC not verified!!");
            error = ESP_FAIL;
        }
        else 
        {
            double temp=0;
            temp=(double)(ds18b20_temperature_lsb+(ds18b20_temperature_msb*256.0))/16.0;
            *temperature_data = temp;
        }
    }
    else 
    {
        error = ESP_FAIL;
    }
    return error;
}


static uint8_t ds18b20_calculate_crc(const uint8_t* data, uint8_t len)
{
    uint8_t crc = 0;
    uint8_t length = len;
    while(length--)
    {
        crc = *data++ ^ crc;  
		crc = pgm_read_byte(dscrc2x16_table + (crc & 0x0f)) ^ pgm_read_byte(dscrc2x16_table + 16 + ((crc >> 4) & 0x0f));
    }
    return crc;
}



