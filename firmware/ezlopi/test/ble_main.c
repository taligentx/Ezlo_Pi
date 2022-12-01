#include <stdio.h>
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "driver/adc.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "mbedtls/config.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "ezlopi_timer.h"
#include "timer_service.h"
#include "gpio_isr_service.h"
#include "ezlopi_event_queue.h"

#include "trace.h"
#include "ezlopi.h"
#include "qt_serial.h"
#include "web_provisioning.h"
#include "ezlopi_ble_gatt_server.h"
#include "gpio_isr_service.h"

static void __blinky(void *pv);

void app_main(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    xTaskCreate(__blinky, "__blinky", 2 * 2048, NULL, 1, NULL);
}

static void __blinky(void *pv)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << GPIO_NUM_2),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    uint32_t state = 0;
    uint32_t count = 0;
    uint32_t free_heap = 0;
    uint32_t min_free_heap = 0;
    gpio_config(&io_conf);

    while (1)
    {
        gpio_set_level(GPIO_NUM_2, state);

        int print = 0;
        if ((free_heap != esp_get_free_heap_size() || (min_free_heap != esp_get_minimum_free_heap_size())) || (count > 5))
        {
            print = 1;
            free_heap = esp_get_free_heap_size();
            min_free_heap = esp_get_minimum_free_heap_size();
        }

        if (print)
        {
            TRACE_D("-----------------------------------------");
            TRACE_D("esp_get_free_heap_size - %d", free_heap);
            TRACE_D("esp_get_minimum_free_heap_size: %u", min_free_heap);
            TRACE_D("-----------------------------------------");
        }

        state ^= 1;
        count++;
        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}
