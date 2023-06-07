#include "string.h"
#include "time.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "trace.h"
#include "ezlopi_nvs.h"
// #include "interface_common.h"

static nvs_handle_t ezlopi_nvs_handle = 0;
static const char *storage_name = "storage";
static const char *config_nvs_name = "config_data";
static const char *passkey_nvs_name = "passkey";
static const char *user_id_nvs_name = "user_id";
static const char *wifi_info_nvs_name = "wifi_info";
static const char *boot_count_nvs_name = "boot_count";
static const char *provisioning_status_nvs_name = "prov_stat";
static const char *settings_initialized_status_name = "settings_status";

static char *ezlopi_nvs_read_str(char *nvs_name);
static int ezlopi_nvs_write_str(char *data, uint32_t len, char *nvs_name);

int ezlopi_nvs_init(void)
{
    int ret = 0;
    if (0 == ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_flash_init();

        if (ESP_ERR_NVS_NO_FREE_PAGES == err || ESP_ERR_NVS_NEW_VERSION_FOUND == err)
        {
            TRACE_D("NVS Init Failed once!, Error: %s", esp_err_to_name(err));
            ESP_ERROR_CHECK(nvs_flash_erase());
                err = nvs_flash_init();
        }

        err = nvs_open(storage_name, NVS_READWRITE, &ezlopi_nvs_handle);
        if (ESP_OK != err)
        {
            TRACE_E("NVS Open Error!");
            // vTaskDelay(2000 / portTICK_RATE_MS);
        }
        else
        {
            ret = 1;
            TRACE_D("NVS Open success");
        }
    }
    else
    {
        ret = 1;
    }

    return ret;
}

int ezlopi_nvs_factory_reset(void)
{
    int ret = 0;
    if (ESP_OK == nvs_flash_erase())
    {
        ret = 1;
    }

    return ret;
}

int ezlopi_nvs_write_config_data_str(char *data)
{
    int ret = 0;
    if (1 == ezlopi_nvs_write_str(data, strlen(data), config_nvs_name))
    {
        ret = 1;
    }
    return ret;
}

char *ezlopi_nvs_read_config_data_str(void)
{
    return ezlopi_nvs_read_str(config_nvs_name);
}

int ezlopi_nvs_read_ble_passkey(uint32_t *passkey)
{
    const uint32_t default_passkey = 123456;
    int ret = 0;
    if (passkey)
    {
        if (1 == ezlopi_nvs_init())
        {
            esp_err_t err = ESP_OK;
            err = nvs_get_u32(ezlopi_nvs_handle, passkey_nvs_name, passkey);
            TRACE_W("nvs_get_u32 - error: %s", esp_err_to_name(err));

            if (ESP_OK == err)
            {
                ret = 1;
            }
            else
            {
                *passkey = 0;
            }
        }

        *passkey = ((0 == *passkey) || (*passkey > 999999)) ? default_passkey : *passkey;
    }

    return ret;
}

int ezlopi_nvs_write_ble_passkey(uint32_t passkey)
{
    int ret = 0;

    if (1 == ezlopi_nvs_init())
    {
        esp_err_t err = ESP_OK;
        err = nvs_set_u32(ezlopi_nvs_handle, passkey_nvs_name, passkey);
        TRACE_W("nvs_set_u32 - error: %s", esp_err_to_name(err));

        if (ESP_OK == err)
        {
            ret = 1;
        }
    }

    return ret;
}

int ezlopi_nvs_write_wifi(const char *wifi_info, uint32_t len)
{
    int ret = 0;
    if (1 == ezlopi_nvs_write_str(wifi_info, len, wifi_info_nvs_name))
    {
        ret = 1;
    }
    return ret;
}

int ezlopi_nvs_read_wifi(char *wifi_info, uint32_t len)
{
    int ret = 0;
    if (1 == ezlopi_nvs_init())
    {
        size_t required_size;
        esp_err_t err = nvs_get_blob(ezlopi_nvs_handle, wifi_info_nvs_name, NULL, &required_size);
        if ((ESP_OK == err) && (len >= required_size))
        {
            err = nvs_get_blob(ezlopi_nvs_handle, wifi_info_nvs_name, wifi_info, &required_size);
            TRACE_W("Error nvs_get_blob: %s", esp_err_to_name(err));
            if (ESP_OK == err)
            {
                ret = 1;
                TRACE_D("Load wifi config:: ssid: %s, password: %s", wifi_info, &wifi_info[32]);
            }
        }
        else
        {
            TRACE_E("'wifi config' read-lenght error!, Required: %d | %d, Error: %s", required_size, len, esp_err_to_name(err));
        }
    }
    return ret;
}

int ezlopi_nvs_write_user_id_str(char *data)
{
    int ret = 0;
    if (1 == ezlopi_nvs_init())
    {
        esp_err_t err;
        err = nvs_set_str(ezlopi_nvs_handle, user_id_nvs_name, data);
        TRACE_W("ezlopi_nvs_write_user_id_str - error: %s", esp_err_to_name(err));

        if (ESP_OK == err)
        {
            err = nvs_commit(ezlopi_nvs_handle);

            if (ESP_OK == err)
            {
                ret = 1;
            }
            else
            {
                TRACE_E("ezlopi_nvs_write_user_id_str - error: %s", esp_err_to_name(err));
            }
        }
    }

    return ret;
}

char *ezlopi_nvs_read_user_id_str(void)
{
    return ezlopi_nvs_read_str(user_id_nvs_name);
}

void ezlopi_nvs_deinit(void)
{
    nvs_close(ezlopi_nvs_handle);
    ezlopi_nvs_handle = 0;
}

void ezlopi_nvs_set_provisioning_status(void)
{
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_set_u32(ezlopi_nvs_handle, provisioning_status_nvs_name, 1);
        TRACE_E("nvs_set_u32 - error: %s", esp_err_to_name(err));
    }
}

uint32_t ezlopi_nvs_get_provisioning_status(void)
{
    uint32_t provisioning_status = 0;
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_get_u32(ezlopi_nvs_handle, provisioning_status_nvs_name, &provisioning_status);
        TRACE_I("Provisioning_Status: %d", provisioning_status);
        if (ESP_OK != err)
        {
            provisioning_status = 0;
            TRACE_E("Error nvs_get_u32: %s", esp_err_to_name(err));
        }
    }

    return provisioning_status;
}

void ezlopi_nvs_set_boot_count(uint32_t boot_count)
{
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_set_u32(ezlopi_nvs_handle, boot_count_nvs_name, boot_count);
        TRACE_W("nvs_set_u32 - error: %s", esp_err_to_name(err));
    }
}

uint32_t ezlopi_nvs_get_boot_count(void)
{
    uint32_t boot_count = 1;
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_get_u32(ezlopi_nvs_handle, boot_count_nvs_name, &boot_count);
        TRACE_I("Boot count: %d", boot_count);
        TRACE_D("Error nvs_get_blob: %s", esp_err_to_name(err));
        if (ESP_OK != err)
        {
            err = nvs_set_u32(ezlopi_nvs_handle, boot_count_nvs_name, boot_count);
            TRACE_W("nvs_set_u32 - error: %s", esp_err_to_name(err));
        }
    }

    return boot_count;
}

uint8_t ezlopi_nvs_get_settings_init_status(void) {
    uint8_t settings_init_status = 0;
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_get_u8(ezlopi_nvs_handle, settings_initialized_status_name, &settings_init_status);
        if (ESP_OK != err)
        {
            TRACE_W("nvs_get_u8 - error: %s", esp_err_to_name(err));
        }
    }

    return settings_init_status;
}

void ezlopi_nvs_set_settings_init_status(void) {
    uint8_t settings_init_status = 1;
    if (ezlopi_nvs_handle)
    {
        esp_err_t err = nvs_set_u8(ezlopi_nvs_handle, settings_initialized_status_name, settings_init_status);
        if (ESP_OK != err)
        {
            TRACE_W("nvs_set_u8 - error: %s", esp_err_to_name(err));
        }
    }
}

int ezlopi_settings_save_settings(const l_ezlopi_settings_t * settings_list)
{
    const l_ezlopi_settings_t* settings_list_current = settings_list;
    while (settings_list_current != NULL) 
    {

        if (settings_list_current->settings_is_enum) 
        {
            ezlopi_nvs_write_str((char *)settings_list_current->settings_value, 
                                strlen(settings_list_current->settings_value), 
                                settings_list_current->settings_name);
            TRACE_I("settings_name: %s, settings_value: %s, settings_value_len: %d", 
                    settings_list_current->settings_name, 
                    (const char*)settings_list_current->settings_value, 
                    strlen((const char*)settings_list_current->settings_value));     
        } 
        else if (strcmp(settings_list_current->settings_value_type, "bool") == 0)
        {            
            nvs_set_u8(ezlopi_nvs_handle, settings_list_current->settings_name, *(bool*)settings_list_current->settings_value ? 1 : 0);
            TRACE_I("settings_name: %s, settings_value: %d", settings_list_current->settings_name, *(bool*)settings_list_current->settings_value ? 1 : 0);
        }
        else if (strcmp(settings_list_current->settings_value_type, "int") == 0) 
        {
            nvs_set_i32(ezlopi_nvs_handle, settings_list_current->settings_name, *((int32_t *)settings_list_current->settings_value));
            TRACE_I("settings_name: %s, settings_value: %d", settings_list_current->settings_name, *(int32_t *)settings_list_current->settings_value);
        }
        else 
        {
            TRACE_W("Unknown settings_value type\n");
        }
        settings_list_current = settings_list_current->next;
    }
    
    return 1;
}

int ezlopi_settings_retrive_settings(l_ezlopi_settings_t * settings_list)
{
    while (settings_list != NULL) 
    {
        // #if 1
        // if (settings_list->settings_is_enum)
        {
            TRACE_I("settings_list->settings_name : %s", settings_list->settings_name);
            char * enum_settings_value = ezlopi_nvs_read_str(settings_list->settings_name);
            if(NULL != enum_settings_value) 
            {
                settings_list->settings_value = (void *)enum_settings_value; // (void *)ezlopi_nvs_read_str(settings_list->settings_name);

                TRACE_I("settings_name: %s, settings_value: %s, settings_value_len: %d", 
                        settings_list->settings_name, 
                        (const char*)settings_list->settings_value, 
                        strlen((const char*)settings_list->settings_value));                  

            }   
        } 
        #if 0
        else if (strcmp(settings_list->settings_value_type, "bool") == 0)
        {            
            uint8_t settings_bool_val;
            bool settings_bool_val_bool;
            nvs_get_u8(ezlopi_nvs_handle, settings_list->settings_name, &settings_bool_val);
            settings_bool_val_bool = (settings_bool_val == 1) ? true : false;
            settings_list->settings_value = (void *)&settings_bool_val_bool;
            TRACE_I("settings_name: %s, settings_value: %d", settings_list->settings_name, *(bool*)settings_list->settings_value ? 1 : 0);
        }
        else if (strcmp(settings_list->settings_value_type, "int") == 0) 
        {
            int32_t settings_int_val = 0;
            nvs_get_i32(ezlopi_nvs_handle, settings_list->settings_name, &settings_int_val);
            settings_list->settings_value = (void *)&settings_int_val;
            TRACE_I("settings_name: %s, settings_value: %d", settings_list->settings_name, *(int32_t *)settings_list->settings_value);
        }
        else 
        {
            TRACE_W("Unknown settings_value type\n");
        }
    #endif
        settings_list = settings_list->next;
    }
    return 1;

}

static int ezlopi_nvs_write_str(char *data, uint32_t len, char *nvs_name)
{
    int ret = 0;

    if (data && nvs_name && len)
    {
        if (1 == ezlopi_nvs_init())
        {
            esp_err_t err = nvs_set_blob(ezlopi_nvs_handle, nvs_name, data, len);
            if (ESP_OK == err)
            {
                err = nvs_commit(ezlopi_nvs_handle);
                if (ESP_OK == err)
                {
                    ret = 1;
                    TRACE_D("%s commit success.", nvs_name);
                }
                else
                {
                    TRACE_E("%s commit error: %s", nvs_name, esp_err_to_name(err));
                }
            }
            else
            {
                TRACE_E("%s write error: %s", nvs_name, esp_err_to_name(err));
            }
        }
    }

    return ret;
}

static char *ezlopi_nvs_read_str(char *nvs_name)
{
    char *return_str = NULL;

    if (1 == ezlopi_nvs_init())
    {
        esp_err_t err = ESP_OK;
        size_t buf_len_needed;
        err = nvs_get_str(ezlopi_nvs_handle, nvs_name, NULL, &buf_len_needed);

        if (buf_len_needed && (ESP_OK == err))
        {
            return_str = malloc(buf_len_needed + 1);

            if (return_str)
            {
                err = nvs_get_str(ezlopi_nvs_handle, nvs_name, return_str, &buf_len_needed);

                if (ESP_OK == err)
                {
                    TRACE_D("%s read success. Data[%d]: %s", nvs_name, buf_len_needed, return_str);
                }
                else
                {
                    TRACE_E("%s read error: %s", nvs_name, esp_err_to_name(err));
                    free(return_str);
                    return_str = NULL;
                }
            }
            else
            {
                TRACE_E("MALLOC ERROR");
            }
        }
    }

    return return_str;
}
