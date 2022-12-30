
#include "sdkconfig.h"

#if (CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3)

#include "ultrasonic_HC_SR04.h"
#include "cJSON.h"
#include "trace.h"
#include "frozen.h"
#include "ezlopi_actions.h"
#include "ezlopi_timer.h"
#include "items.h"

#include "ezlopi_devices_list.h"
#include "ezlopi_device_value_updated.h"
#include "ezlopi_cloud_constants.h"
#include "stdlib.h"

#include "driver/mcpwm.h"
#include "soc/rtc.h"

static xQueueHandle ezlopi_ultrasonic_HC_SR04_queue = NULL;
static bool current_val = false;
static bool previous_val = false;

static int ezlopi_ultrasonic_HC_SR04_prepare_and_add(void *args);
static s_ezlopi_device_properties_t *ezlopi_ultrasonic_HC_SR04_prepare(cJSON *cjson_device);
static int ezlopi_ultrasonic_HC_SR04_init(s_ezlopi_device_properties_t *properties);
static int ezlopi_ultrasonic_HC_SR04_get_value_cjson(s_ezlopi_device_properties_t *properties, void *args);
static bool ezlopi_ultrasonic_HC_SR04_upcall(mcpwm_unit_t unit, mcpwm_capture_channel_id_t channel, const cap_event_data_t *data, void *user_data);
static bool ezlopi_ultrasonic_HC_SR04_get_from_sensor(s_ezlopi_device_properties_t *properties);

int ultrasonic_HC_SR04(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg)
{
    int ret = 0;

    switch (action)
    {
    case EZLOPI_ACTION_PREPARE:
    {
        ret = ezlopi_ultrasonic_HC_SR04_prepare_and_add(arg);
        break;
    }
    case EZLOPI_ACTION_INITIALIZE:
    {
        ret = ezlopi_ultrasonic_HC_SR04_init(properties);
        break;
    }
    case EZLOPI_ACTION_NOTIFY_200_MS:
    {
        ret = ezlopi_ultrasonic_HC_SR04_get_from_sensor(properties);
        break;
    }
    case EZLOPI_ACTION_GET_EZLOPI_VALUE:
    {
        ezlopi_ultrasonic_HC_SR04_get_value_cjson(properties, arg);
        break;
    }
    default:
    {
        break;
    }
    }

    return ret;
}

static int ezlopi_ultrasonic_HC_SR04_prepare_and_add(void *args)
{
    int ret = 0;
    s_ezlopi_prep_arg_t *device_prep_arg = (s_ezlopi_prep_arg_t *)args;

    if ((NULL != device_prep_arg) && (NULL != device_prep_arg->cjson_device))
    {
        s_ezlopi_device_properties_t *ezlopi_ultrasonic_HC_SR04_properties = ezlopi_ultrasonic_HC_SR04_prepare(device_prep_arg->cjson_device);
        if (ezlopi_ultrasonic_HC_SR04_properties)
        {
            if (0 == ezlopi_devices_list_add(device_prep_arg->device, ezlopi_ultrasonic_HC_SR04_properties, NULL))
            {
                free(ezlopi_ultrasonic_HC_SR04_properties);
            }
            else
            {
                ret = 1;
            }
        }
    }

    return ret;
}

static s_ezlopi_device_properties_t *ezlopi_ultrasonic_HC_SR04_prepare(cJSON *cjson_device)
{
    s_ezlopi_device_properties_t *ezlopi_ultrasonic_HC_SR04_properties = malloc(sizeof(s_ezlopi_device_properties_t));

    if (ezlopi_ultrasonic_HC_SR04_properties)
    {
        memset(ezlopi_ultrasonic_HC_SR04_properties, 0, sizeof(s_ezlopi_device_properties_t));
        ezlopi_ultrasonic_HC_SR04_properties->interface_type = EZLOPI_DEVICE_INTERFACE_DIGITAL_OUTPUT;

        char *device_name = NULL;
        CJSON_GET_VALUE_STRING(cjson_device, "dev_name", device_name);
        ASSIGN_DEVICE_NAME(ezlopi_ultrasonic_HC_SR04_properties, device_name);
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.category = category_security_sensor;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.subcategory = subcategory_motion;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.item_name = ezlopi_item_name_motion;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.device_type = dev_type_sensor_motion;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.value_type = value_type_bool;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.has_getter = true;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.has_setter = false;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.reachable = true;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.battery_powered = false;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.show = true;
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.room_name[0] = '\0';
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.device_id = ezlopi_device_generate_device_id();
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.room_id = ezlopi_device_generate_room_id();
        ezlopi_ultrasonic_HC_SR04_properties->ezlopi_cloud.item_id = ezlopi_device_generate_item_id();

        // CJSON_GET_VALUE_INT(cjson_device, "gpio1", ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.gpio_num);
        // CJSON_GET_VALUE_INT(cjson_device, "gpio2", ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.gpio_num);

        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.gpio_num = 9;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.enable = true;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.interrupt = GPIO_INTR_DISABLE;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.invert = EZLOPI_GPIO_LOGIC_NONINVERTED;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.mode = GPIO_MODE_OUTPUT;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.pull = GPIO_PULLDOWN_ONLY;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_out.value = 0;

        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.gpio_num = 10;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.enable = true;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.interrupt = GPIO_INTR_DISABLE;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.invert = EZLOPI_GPIO_LOGIC_NONINVERTED;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.mode = GPIO_MODE_INPUT;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.pull = GPIO_PULLDOWN_ONLY;
        ezlopi_ultrasonic_HC_SR04_properties->interface.gpio.gpio_in.value = 0;
    }
    return ezlopi_ultrasonic_HC_SR04_properties;
}

static int ezlopi_ultrasonic_HC_SR04_init(s_ezlopi_device_properties_t *properties)
{
    int ret = -1;
    if (GPIO_IS_VALID_GPIO(properties->interface.gpio.gpio_out.gpio_num))
    {
        const gpio_config_t io_conf = {
            .pin_bit_mask = (1ULL << properties->interface.gpio.gpio_out.gpio_num),
            .mode = GPIO_MODE_OUTPUT,
            .pull_up_en = GPIO_PULLUP_DISABLE,
            .pull_down_en = GPIO_PULLDOWN_ENABLE,
            .intr_type = GPIO_INTR_DISABLE,
        };

        gpio_config(&io_conf);
        gpio_set_level(properties->interface.gpio.gpio_out.gpio_num, 0);
    }
    if (GPIO_IS_VALID_GPIO(properties->interface.gpio.gpio_in.gpio_num))
    {
        ezlopi_ultrasonic_HC_SR04_queue = xQueueCreate(1, sizeof(uint32_t));
        mcpwm_unit_t unit = MCPWM_UNIT_0;
        mcpwm_io_signals_t io_signal = MCPWM_CAP_0;
        int ultrasonic_gpio = GPIO_NUM_10;
        mcpwm_capture_channel_id_t channel_id = MCPWM_SELECT_CAP0;

        const mcpwm_capture_config_t capture_conf = {
            .cap_edge = MCPWM_BOTH_EDGE,
            .cap_prescale = 1,
            .capture_cb = ezlopi_ultrasonic_HC_SR04_upcall,
            .user_data = NULL,
        };

        esp_err_t error = mcpwm_gpio_init(unit, io_signal, ultrasonic_gpio);
        if (error)
        {
            TRACE_E("Error initializing gpio.(%s)", esp_err_to_name(error));
        }
        else
        {
            TRACE_I("GPIO was initialized successfully.");
        }

        error = mcpwm_capture_enable_channel(unit, channel_id, &capture_conf);
        if (error)
        {
            TRACE_E("Error enabling capture channel.(%s)", esp_err_to_name(error));
        }
        else
        {
            TRACE_I("Capture channel was enabled successfully.");
        }
    }
    return ret;
}

static bool ezlopi_ultrasonic_HC_SR04_upcall(mcpwm_unit_t unit, mcpwm_capture_channel_id_t channel, const cap_event_data_t *data, void *user_data)
{
    static uint32_t start_of_sample = 0;
    static uint32_t end_of_sample = 0;

    if (MCPWM_POS_EDGE == data->cap_edge)
    {
        start_of_sample = data->cap_value;
    }
    else if (MCPWM_NEG_EDGE == data->cap_edge)
    {
        end_of_sample = data->cap_value;
        uint32_t pulse_count = end_of_sample - start_of_sample;
        xQueueSendFromISR(ezlopi_ultrasonic_HC_SR04_queue, &pulse_count, NULL);
    }
    return pdTRUE;
}

static bool ezlopi_ultrasonic_HC_SR04_get_from_sensor(s_ezlopi_device_properties_t *properties)
{
    uint32_t pulse_count;
    float distance;
    bool ret = false;
    ESP_ERROR_CHECK(gpio_set_level(properties->interface.gpio.gpio_out.gpio_num, 1));
    vTaskDelay(1 / portTICK_PERIOD_MS);
    ESP_ERROR_CHECK(gpio_set_level(properties->interface.gpio.gpio_out.gpio_num, 0));
    if (xQueueReceive(ezlopi_ultrasonic_HC_SR04_queue, &pulse_count, portMAX_DELAY))
    {
        uint32_t pulse_width = pulse_count * (1000000.0 / rtc_clk_apb_freq_get());
        distance = (float)(pulse_width / 58);
        if (distance < 60)
        {
            current_val = true;
        }
        else
        {
            current_val = false;
        }
        if (current_val != previous_val)
        {
            ezlopi_device_value_updated_from_device(properties);
            previous_val = current_val;
        }
    }
    return ret;
}

static int ezlopi_ultrasonic_HC_SR04_get_value_cjson(s_ezlopi_device_properties_t *properties, void *args)
{
    int ret = 0;

    cJSON *cjson_propertise = (cJSON *)args;
    if (cjson_propertise)
    {
        cJSON_AddBoolToObject(cjson_propertise, "value", current_val);
        ret = 1;
    }
    return ret;
}

#endif // CONFIG_IDF_TARGET_ESP32 || CONFIG_IDF_TARGET_ESP32S3