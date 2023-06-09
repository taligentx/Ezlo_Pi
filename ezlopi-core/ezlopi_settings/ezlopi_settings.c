#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ezlopi_settings.h"
#include "ezlopi_nvs.h"
#include "trace.h"

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

uint16_t ezlopi_settings_get_settings_count(void) {
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

// Function to print the settings in JSON format
void ezlopi_settings_print_settings() {
    printf("{\n");
    printf("  \"settings\": [\n");

    for (size_t i = 0; i < ezlopi_settings_get_settings_count(); i++) {
        printf("    {\n");
        printf("      \"enum\": [\n");
        if (ezlopi_settings_list[i].enum_values[0] != NULL) {
            for (size_t j = 0; j < EZLOPI_SETTINGS_MAX_ENUM_VALUES && ezlopi_settings_list[i].enum_values[j] != NULL; j++) {
                printf("        \"%s\"", ezlopi_settings_list[i].enum_values[j]);
                if (j < EZLOPI_SETTINGS_MAX_ENUM_VALUES - 1 && ezlopi_settings_list[i].enum_values[j + 1] != NULL) {
                    printf(",");
                }
                printf("\n");
            }
        }
        printf("      ],\n");
        printf("      \"name\": \"%s\",\n", ezlopi_settings_list[i].name);
        printf("      \"value\": ");
        switch (ezlopi_settings_list[i].value_type) {
            case EZLOPI_SETTINGS_TYPE_TOKEN:
                printf("\"%s\",\n", ezlopi_settings_list[i].value.token_value);
                break;
            case EZLOPI_SETTINGS_TYPE_BOOL:
                printf("%s,\n", ezlopi_settings_list[i].value.bool_value ? "true" : "false");
                break;
            case EZLOPI_SETTINGS_TYPE_INT:
                printf("%d,\n", ezlopi_settings_list[i].value.int_value);
                break;
            default:
                break;                
        }
        printf("      \"valueType\": \"%s\"\n", ezlopi_settings_list[i].value_type == EZLOPI_SETTINGS_TYPE_INT ? "int" : "token");
        printf("    }%s\n", i < sizeof(ezlopi_settings_list) / sizeof(ezlopi_settings_list[0]) - 1 ? "," : "");
    }

    printf("  ]\n");
    printf("}\n");
}

void ezlopi_initialize_settings(void) {

    #if 0

    ezlopi_settings_print_settings();
    printf("---------------------------------------------------------------------------------");
    // Modify a setting
    const char* newTokenValue = "ddmmyy";
    ezlopi_settings_modify_setting("date.format", newTokenValue);
    bool bool_val = true;
    const bool* newBoolValue = &bool_val;
    ezlopi_settings_modify_setting("logs.color", &newBoolValue);
    ezlopi_settings_print_settings();
    printf("---------------------------------------------------------------------------------");
    // Read a setting
    bool boolValue;
    ezlopi_settings_read_setting("logs.color", &boolValue);
    printf("\nlogs.color: %s\n", boolValue ? "true" : "false");
    ezlopi_settings_read_setting("date.format", &newTokenValue);
    printf("\ndate.format: %s\n", newTokenValue);
    int val_int = 0;
    ezlopi_settings_read_setting("some_integer_value", &val_int);
    printf("\nsome_integer_value: %d\n", val_int);    
    printf("---------------------------------------------------------------------------------");
    // Print settings in JSON format
    ezlopi_settings_print_settings(); 

    #endif 
    ezlopi_settings_save_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
    ezlopi_settings_print_settings();
    ezlopi_settings_retrive_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
    ezlopi_settings_print_settings();

    // if(EZLOPI_SETTINGS_INITI_STATUS_FALSE == ezlopi_nvs_get_settings_init_status())
    // {
    //     ezlopi_settings_save_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
    //     // ezlopi_settings_print_settings(settings_list);
    //     ezlopi_nvs_set_settings_init_status();
    // }
        
    // if(EZLOPI_SETTINGS_INITI_STATUS_TRUE == ezlopi_nvs_get_settings_init_status()) 
    // {
    //     ezlopi_settings_retrive_settings(ezlopi_settings_list, ezlopi_settings_get_settings_count());
    // }


}

#if 0
void ezlopi_initialize_settings(void) 
{


    // Create settings linked list
    l_ezlopi_settings_t* settings_list = NULL;

    const char* ezlopi_settings_enum_value_date_format[] = { "mmddyy", "ddmmyy" };
    const uint16_t ezlopi_settings_enum_count_date_format = sizeof(ezlopi_settings_enum_value_date_format) / sizeof(ezlopi_settings_enum_value_date_format[0]);
    const char* ezlopi_settings_enum_value_date_format_default = ezlopi_settings_enum_value_date_format[0];
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_date_format, (void*)ezlopi_settings_enum_value_date_format_default, ezlopi_settings_enum_value_date_format, ezlopi_settings_enum_count_date_format, "token", true));

    bool firstStart = true;
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_first_start, (void*)&firstStart, NULL, 0, "bool", false));
    
    const char* ezlopi_settings_enum_time_format[] = { "12", "24" };
    const uint16_t ezlopi_settings_enum_count_time_format = sizeof(ezlopi_settings_enum_time_format) / sizeof(ezlopi_settings_enum_time_format[0]);
    const char* ezlopi_settings_enum_time_format_default = ezlopi_settings_enum_time_format[0];
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_time_format, (void*)ezlopi_settings_enum_time_format_default, ezlopi_settings_enum_time_format, ezlopi_settings_enum_count_time_format, "token", true));

    
    if(EZLOPI_SETTINGS_INITI_STATUS_FALSE == ezlopi_nvs_get_settings_init_status())
    {
        ezlopi_settings_save_settings(settings_list);
        // ezlopi_settings_print_settings(settings_list);
        ezlopi_nvs_set_settings_init_status();
    }
        
    if(EZLOPI_SETTINGS_INITI_STATUS_TRUE == ezlopi_nvs_get_settings_init_status()) 
    {
        ezlopi_settings_retrive_settings(settings_list);
    }

    // Print settings
    ezlopi_settings_print_settings(settings_list);
    ezlopi_settings_modify_setting(settings_list, str_ezlopi_settings_date_format, ezlopi_settings_enum_value_date_format[1]);
    firstStart = false;
    ezlopi_settings_modify_setting(settings_list, str_ezlopi_settings_first_start, (void *)&firstStart);
    ezlopi_settings_modify_setting(settings_list, str_ezlopi_settings_time_format, ezlopi_settings_enum_time_format[1]);
    ezlopi_settings_print_settings(settings_list);

    void* targetValue = NULL;
    // bool read_data_firstStart = true;
    ezlopi_settings_get_setting(settings_list, str_ezlopi_settings_first_start, (void *)&targetValue);
    TRACE_D("read_data_firstStart: %s", *(bool *)targetValue ? "TRUE" : "FALSE");

    // char read_date_format[50];
    targetValue = NULL;
    ezlopi_settings_get_setting(settings_list, str_ezlopi_settings_date_format, (void *)&targetValue);
    TRACE_D("read_date_format: %s", (const char *)targetValue);

    targetValue = NULL;
    ezlopi_settings_get_setting(settings_list, str_ezlopi_settings_time_format, (void *)&targetValue);
    TRACE_D("read_time_format: %s", (const char *)targetValue);

    // Clean up
    ezlopi_settings_free_settings(&settings_list);

    return 0;
}
#endif 