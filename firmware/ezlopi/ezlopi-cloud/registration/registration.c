#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "trace.h"
#include "registration.h"
#include "ezlopi_websocket_client.h"

static volatile uint32_t is_registered = 0;
static void registration_process(void *pv);

void registration_init(void)
{
    is_registered = 0;
    xTaskCreate(registration_process, "registration_process", 2 * 2048, NULL, 2, NULL);
}

char *registered(const char *payload, uint32_t len, struct json_token *method, uint32_t msg_count)
{
    char *ret = NULL;
    TRACE_I("Device registration successful.");
    is_registered = 1;
    return ret;
}

static void registration_process(void *pv)
{
    char mac_str[18];
    uint8_t mac_addr[6];
    char reg_str[300] = "";

    esp_read_mac(mac_addr, ESP_MAC_WIFI_STA);
    snprintf(mac_str, sizeof(mac_str), "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

    snprintf(reg_str, sizeof(reg_str),
             "{\"id\":\"%u\",\"method\":\"register\",\"params\":"
             "{\"firmware\":\"0.1\",\"timeOffset\":18000, \"media\":\"radio\","
             "\"hubType\":\"32.1\",\"mac_address\":\"%s\",\"maxFrameSize\":2048}}",
             esp_random(), "a2:97:1e:74:0b:52");

    while (false == ezlopi_websocket_client_is_connected())
    {
        vTaskDelay(200 / portTICK_RATE_MS);
    }

    while (0 == is_registered)
    {
        ezlopi_websocket_client_send(reg_str, strlen(reg_str));
        vTaskDelay(2000 / portTICK_RATE_MS);
    }

    vTaskDelete(NULL);
}
