#ifndef __EZLOPI_BLE_SERVICE_H__
#define __EZLOPI_BLE_SERVICE_H__

#define CHECK_PRINT_ERROR(x, msg)                                    \
    {                                                                \
        if (x)                                                       \
        {                                                            \
            TRACE_E("%s %s: %s", __func__, msg, esp_err_to_name(x)); \
            return;                                                  \
        }                                                            \
    }

#define WIFI_CREDS_SERVICE_HANDLE 0
#define WIFI_STATUS_SERVICE_HANDLE 1
#define WIFI_ERROR_SERVICE_HANDLE 2
#define BLE_PASSKEY_SERVICE_HANDLE 3
#define BLE_USER_ID_SERVICE_HANDLE 4
#define BLE_PROVISIONING_ID_HANDLE 5

void ezlopi_ble_service_init(void);

#endif //  __EZLOPI_BLE_SERVICE_H__
