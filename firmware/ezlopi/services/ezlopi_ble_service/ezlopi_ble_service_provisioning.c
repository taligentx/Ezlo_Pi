#include "string.h"

#include "cJSON.h"
#include "lwip/ip_addr.h"
#include "esp_event_base.h"

#include "trace.h"
#include "ezlopi_wifi.h"

#include "ezlopi_nvs.h"
#include "ezlopi_ble_gatt.h"
#include "ezlopi_ble_profile.h"

#include "ezlopi_ble_service.h"
#include "ezlopi_ble_buffer.h"
#include "mbedtls/base64.h"

static s_gatt_service_t *g_provisioning_service;
static s_linked_buffer_t *Certs_creds_linked_buffer = NULL;

static void provisioning_info_write_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param);
static void provisioning_info_read_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param);
static void provisioning_info_exec_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param);

void ezlopi_ble_service_provisioning_init(void)
{
    esp_bt_uuid_t uuid;
    esp_gatt_perm_t permission;
    esp_gatt_char_prop_t properties;

    uuid.len = ESP_UUID_LEN_16;
    uuid.uuid.uuid16 = 0xFF00;
    g_provisioning_service = ezlopi_ble_gatt_create_service(BLE_PROVISIONING_ID_HANDLE, &uuid);

    uuid.uuid.uuid16 = 0xFF01;
    uuid.len = ESP_UUID_LEN_16;
    permission = ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE;
    properties = ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE;
    ezlopi_ble_gatt_add_characteristic(g_provisioning_service, &uuid, permission, properties, provisioning_info_read_func, provisioning_info_write_func, provisioning_info_exec_func);
}

static void provisioning_info_write_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param)
{
    TRACE_D("Write function called!");
    if (0 == param->write.is_prep) // Data received in single packet
    {
        dump("GATT_WRITE_EVT value", param->write.value, 0, param->write.len);
        if ((NULL != param->write.value) && (param->write.len > 0))
        {
            // Certs_creds_parse_and_save(param->write.value, param->write.len);
        }
    }
    else
    {
        if (NULL == Certs_creds_linked_buffer)
        {
            Certs_creds_linked_buffer = ezlopi_ble_buffer_create(param);
        }
        else
        {
            ezlopi_ble_buffer_add_to_buffer(Certs_creds_linked_buffer, param);
        }
    }
}

static void provisioning_info_read_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param)
{
}

static void provisioning_info_exec_func(esp_gatt_value_t *value, esp_ble_gatts_cb_param_t *param)
{
    TRACE_D("Write execute function called.");
    ezlopi_ble_buffer_accumulate_to_start(Certs_creds_linked_buffer);
    // Certs_creds_parse_and_save(Certs_creds_linked_buffer->buffer, Certs_creds_linked_buffer->len);
    ezlopi_ble_buffer_free_buffer(Certs_creds_linked_buffer);
    Certs_creds_linked_buffer = NULL;
}

// static void Certs_creds_parse_and_save(uint8_t *value, uint32_t len)
// {
//     if ((NULL != value) && (len > 0))
//     {
//         cJSON *root = cJSON_Parse((const char *)value);
//         if (root)
//         {
//             printf("value = %s\n", value);
//             char *id = cJSON_GetObjectItemCaseSensitive(root, "id")->valuestring;
//             char *uuid = cJSON_GetObjectItemCaseSensitive(root, "uuid")->valuestring;
//             char *cloud_uuid = cJSON_GetObjectItemCaseSensitive(root, "cloud_uuid")->valuestring;
//             char *order_uuid = cJSON_GetObjectItemCaseSensitive(root, "order_uuid")->valuestring;
//             char *zwave_region = cJSON_GetObjectItemCaseSensitive(root, "zwave_region")->valuestring;
//             char *provision_server = cJSON_GetObjectItemCaseSensitive(root, "provision_server")->valuestring;
//             char *cloud_server = cJSON_GetObjectItemCaseSensitive(root, "cloud_server")->valuestring;
//             char *ssl_private_key = cJSON_GetObjectItemCaseSensitive(root, "ssl_private_key")->valuestring;
//             char *ssl_public_key = cJSON_GetObjectItemCaseSensitive(root, "ssl_public_key")->valuestring;
//             char *ssl_shared_key = cJSON_GetObjectItemCaseSensitive(root, "ssl_shared_key")->valuestring;
//             char *signing_ca_certificate = cJSON_GetObjectItemCaseSensitive(root, "signing_ca_certificate")->valuestring;
//             size_t olen, slen, dlen;
//             slen = strlen(signing_ca_certificate);
//             dlen = slen; // dlen = slen * 3 / 4;
//             char *dstbuf = malloc(dlen);
//             memset(dstbuf, 0, dlen);
//             mbedtls_base64_decode((unsigned char *)dstbuf, dlen, &olen, (const unsigned char *)signing_ca_certificate, slen);

//             cJSON_Delete(root);
//         }
//     }
// }
