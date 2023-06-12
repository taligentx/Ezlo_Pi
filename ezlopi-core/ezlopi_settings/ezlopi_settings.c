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
        .value.bool_value = false,
        .value_type = EZLOPI_SETTINGS_TYPE_BOOL
    },
    {
        .enum_values = {"12", "24"},
        .name = "time.format",
        .value.token_value = "12",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    }
};

s_ezlopi_settings_t ezlopi_settings_list_ii[] = {
    {
        .enum_values = {"mmddyy", "ddmmyy"},
        .name = "date.format",        
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .name = "first_start",
        .value_type = EZLOPI_SETTINGS_TYPE_BOOL
    },
    {
        .enum_values = {"12", "24"},
        .name = "time.format",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    }
};

static void modify_setting(const char* name, const void* value)
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


static void read_setting(const char* name, void* value) 
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


static void print_settings(const s_ezlopi_settings_t *settings_list, size_t num_settings) {
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

s_ezlopi_settings_t *ezlopi_settings_get_settings_list(void)
{
    return ezlopi_settings_list;
}

uint16_t ezlopi_settings_get_settings_count(void) 
{
    return sizeof(ezlopi_settings_list) / sizeof(ezlopi_settings_list[0]);
}

int ezlopi_settings_modify_setting(const char* name, const void* value)
{
    int ret = 1;
    modify_setting(name, value);
    if(ezlopi_settings_save_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count()))  
    {
        TRACE_D("Seetings modified, Setting name: %s", name);
    }
    else 
    {
        TRACE_E("Failed settings modification, Setting name: %s", name);
        ret = 0;
    }

    print_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());     
    return ret;
}

uint8_t ezlopi_settings_read_setting(const char* name, void* value)
{    
    int ret = 0;

    if(ezlopi_nvs_get_settings_init_status()) 
    {
        if(ezlopi_settings_retrive_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count()))
        {
            read_setting(name, value);
            TRACE_D("Seetings retrived from NVS.");
            ret = 1;
        }
        else
        {
            TRACE_E("Failed retriving settings %s from NVS!", name);
            ret = 0;
        }
    }
    return ret;
}

void ezlopi_initialize_settings(void) 
{
    if(!ezlopi_nvs_get_settings_init_status())
    {
        if(ezlopi_settings_save_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count()))  
        {
            if(ezlopi_nvs_set_settings_init_status()) 
            {
                TRACE_D("Seetings Initialized to NVS.");
            } 
            else 
            {
                TRACE_E("Failed saving settings status to NVS!");
            }
        }
        else 
        {
            TRACE_E("Failed saving settings to NVS!");
        }

        print_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());               
    }
    
    if(ezlopi_nvs_get_settings_init_status()) 
    {
        if(ezlopi_settings_retrive_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count()))
        {
            TRACE_D("Seetings retrived from NVS.");
            print_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
        }
        else
        {
            TRACE_E("Failed retriving settings from NVS!");
        }
    }
    void *str_val = NULL;
    bool bool_val;
    // bool *int_val = NULL;
    ezlopi_settings_read_setting("date.format", (void *)&str_val);
    TRACE_D("Seetings, \"date.format\" : %s", (const char *)str_val);
    ezlopi_settings_read_setting("first_start", (void *)&bool_val);
    TRACE_D("Seetings, \"first_start\" : %s", bool_val ? "TRUE" : "FALSE");    


}

