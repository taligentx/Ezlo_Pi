#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ezlopi_settings.h"
#include "ezlopi_nvs.h"
#include "trace.h"
#include "cJSON.h"

s_ezlopi_settings_t ezlopi_settings_list[] = {
    {
        .enum_values = {"mmddyy", "ddmmyy"},
        .name = "date.format",
        .value.token_value = "mmddyy",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .name = "first_start",
        .value.bool_value = true,
        .value_type = EZLOPI_SETTINGS_TYPE_BOOL
    },
    {
        .enum_values = {"12", "24"},
        .name = "time.format",
        .value.token_value = "12",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    }
};

s_ezlopi_settings_t *ezlopi_settings_get_settings_list(void)
{
    return ezlopi_settings_list;
}

uint16_t ezlopi_settings_get_settings_count(void) 
{
    return sizeof(ezlopi_settings_list) / sizeof(ezlopi_settings_list[0]);
}

void ezlopi_settings_modify_setting(const char* name, const void* value) 
{
    for (size_t i = 0; i < ezlopi_settings_get_settings_count(); i++) 
    {
        if (strcmp(ezlopi_settings_list[i].name, name) == 0) 
        {
            switch (ezlopi_settings_list[i].value_type) 
            {
                case EZLOPI_SETTINGS_TYPE_TOKEN:
                    ezlopi_settings_list[i].value.token_value = (const char*)value;
                    break;
                case EZLOPI_SETTINGS_TYPE_BOOL:
                    ezlopi_settings_list[i].value.bool_value = *((bool*)value);
                    break;
                case EZLOPI_SETTINGS_TYPE_INT:
                    ezlopi_settings_list[i].value.int_value = *((int*)value);
                    break;
                default:
                    break;
            }
            break;
        }
    }
}

// Function to read the value of a single setting
void ezlopi_settings_read_setting(const char* name, void* value) 
{
    for (size_t i = 0; i < ezlopi_settings_get_settings_count(); i++) {
        if (strcmp(ezlopi_settings_list[i].name, name) == 0) {
            switch (ezlopi_settings_list[i].value_type) {
                case EZLOPI_SETTINGS_TYPE_TOKEN:
                    *((const char**)value) = ezlopi_settings_list[i].value.token_value;
                    break;
                case EZLOPI_SETTINGS_TYPE_BOOL:
                    *((bool*)value) = ezlopi_settings_list[i].value.bool_value;
                    break;
                case EZLOPI_SETTINGS_TYPE_INT:
                    *((int*)value) = ezlopi_settings_list[i].value.int_value;
                    break;
                default:
                    break;                    
            }
            break;
        }
    }
}


void ezlopi_settings_print_settings(const s_ezlopi_settings_t *settings_list, size_t num_settings) {
    cJSON *root = cJSON_CreateArray();

    for (size_t i = 0; i < num_settings; i++) {
        cJSON *setting = cJSON_CreateObject();
        cJSON_AddStringToObject(setting, "name", settings_list[i].name);
        
        switch (settings_list[i].value_type) {
            case EZLOPI_SETTINGS_TYPE_TOKEN:
                cJSON_AddStringToObject(setting, "value", settings_list[i].value.token_value);
                cJSON_AddStringToObject(setting, "valueType", "token");
                break;
            case EZLOPI_SETTINGS_TYPE_BOOL:
                cJSON_AddBoolToObject(setting, "value", settings_list[i].value.bool_value);
                cJSON_AddStringToObject(setting, "valueType", "bool");
                break;
            case EZLOPI_SETTINGS_TYPE_INT:
                cJSON_AddNumberToObject(setting, "value", settings_list[i].value.int_value);
                cJSON_AddStringToObject(setting, "valueType", "int");
                break;
            default:
                cJSON_Delete(setting);
                continue;
        }
        
        cJSON_AddItemToArray(root, setting);
    }

    char *json_str = cJSON_PrintUnformatted(root);
    printf("%s\n", json_str);
    cJSON_free(json_str);
    cJSON_Delete(root);
}


void ezlopi_initialize_settings(void) {


    if(EZLOPI_SETTINGS_INITI_STATUS_FALSE == ezlopi_nvs_get_settings_init_status())
    {
        ezlopi_settings_save_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
        ezlopi_settings_print_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
        ezlopi_nvs_set_settings_init_status();
    }
        
    if(EZLOPI_SETTINGS_INITI_STATUS_TRUE == ezlopi_nvs_get_settings_init_status()) 
    {
        ezlopi_settings_retrive_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
        ezlopi_settings_print_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
    }

}

