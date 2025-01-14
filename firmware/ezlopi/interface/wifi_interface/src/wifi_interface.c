/* WiFi station Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_event_base.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "esp_netif_types.h"
// #include "esp_netif_lwip_internal.h"

#include "debug.h"
#include "wifi_interface.h"
#include "factory_info.h"
#include "nvs_storage.h"
// #include "qt_serial.h"

/* The examples use WiFi configuration that you can set via project configuration menu

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/
#define EXAMPLE_ESP_WIFI_SSID "nepaldigisys"
#define EXAMPLE_ESP_WIFI_PASS "NDS_0ffice"
#define EXAMPLE_ESP_MAXIMUM_RETRY 5

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;
static esp_netif_t *wifi_sta_netif = NULL;
static esp_netif_ip_info_t my_ip;

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

static uint32_t new_wifi = 0;
static int s_retry_num = 0;
static char wifi_ssid_pass[64];
static int station_got_ip = 0;

int qt_serial_respond_to_qt(int len, uint8_t *data);

esp_netif_ip_info_t *wifi_get_ip_infos(void)
{
    return &my_ip;
}

static void alert_qt_wifi_fail(void)
{
    char *qt_resp = "{\"cmd\":2,\"status_write\":0,\"status_connect\":0}";
    qt_serial_respond_to_qt(strlen(qt_resp), (uint8_t *)qt_resp);
}

// extern "C" void set_new_wifi_flag_c(void);
void set_new_wifi_flag_c(void)
{
    new_wifi = 1;
}

// extern "C" char *get_current_wifi_creds_c(void);
char *get_current_wifi_creds_c(void)
{
    return wifi_ssid_pass;
}

// extern "C" int got_ip_c(void);
int got_ip_c(void)
{
    return station_got_ip;
}

// extern "C" void wifi_connect_c(const char *ssid, const char *pass);
void wifi_connect_c(const char *ssid, const char *pass)
{
    wifi_connect(ssid, pass);
}

void set_new_wifi_flag(void)
{
    new_wifi = 1;
}

static void set_wifi_station_host_name(void)
{
    static char station_host_name[32];
    // factory_info *factory = factory_info::get_instance();
    s_factory_info_t *factory = factory_info_get_info();
    snprintf(station_host_name, sizeof(station_host_name), "EZLOPI-%llu", factory->id);
    esp_err_t err = tcpip_adapter_set_hostname(TCPIP_ADAPTER_IF_STA, station_host_name);
    TRACE_W("'tcpip_adapter_set_hostname' ERROR: %s", esp_err_to_name(err));
}

static void alert_qt_wifi_got_ip(void)
{
    // qt_serial *qt_ctx = qt_serial::get_instance();

    if (new_wifi)
    {
        new_wifi = 0;
        nvs_storage_write_wifi(wifi_ssid_pass, sizeof(wifi_ssid_pass));

        char *qt_resp = "{\"cmd\":2,\"status_write\":1,\"status_connect\":1}";
        qt_serial_respond_to_qt(strlen(qt_resp), (uint8_t *)qt_resp);
    }
    else
    {

        char *qt_resp = "{\"cmd\":2,\"status_connect\":1}";
        qt_serial_respond_to_qt(strlen(qt_resp), (uint8_t *)qt_resp);
    }
}

static void event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        station_got_ip = 0;
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY)
        {
            esp_wifi_connect();
            s_retry_num++;
            TRACE_I("retry to connect to the AP");
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            alert_qt_wifi_fail();
            s_retry_num = 0;
        }
        TRACE_I("connect to the AP fail");
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        TRACE_I("got - ip:      " IPSTR, IP2STR(&event->ip_info.ip));
        TRACE_I("      netmask: " IPSTR, IP2STR(&event->ip_info.netmask));
        TRACE_I("      gw:      " IPSTR, IP2STR(&event->ip_info.gw));

        station_got_ip = 1;

        s_retry_num = 0;
        memcpy(&my_ip, &event->ip_info, sizeof(esp_netif_ip_info_t));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        alert_qt_wifi_got_ip();
    }
}

void wifi_initialize(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_sta_netif = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &event_handler, NULL, &instance_got_ip));
}

void wifi_connect_from_nvs(void)
{
    char wifi_info[64];
    memset(wifi_info, 0, sizeof(wifi_info));
    nvs_storage_read_wifi(wifi_info, sizeof(wifi_info));

    if (wifi_info[0] == 0)
    {
        strcpy(wifi_info, "krishna home_wlink");
        strcpy(&wifi_info[32], "coldWinter");
    }
    wifi_connect(&wifi_info[0], &wifi_info[32]);
}

void wifi_connect(const char *ssid, const char *pass)
{
    strncpy((char *)&wifi_ssid_pass[0], ssid, 32);
    strncpy((char *)&wifi_ssid_pass[32], pass, 32);

    TRACE_D("SSID: %s, Password: %s\r\n", ssid, pass);

    wifi_config_t wifi_config = {
        .sta = {
            .pmf_cfg = {.capable = true, .required = false},
        },
    };

    strncpy((char *)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char *)wifi_config.sta.password, pass, sizeof(wifi_config.sta.password));
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;

    esp_wifi_stop();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());
    set_wifi_station_host_name();
}

void wait_for_wifi_to_connect(void)
{
    xEventGroupWaitBits(s_wifi_event_group, WIFI_CONNECTED_BIT, pdFALSE, pdFALSE, portMAX_DELAY);
}
