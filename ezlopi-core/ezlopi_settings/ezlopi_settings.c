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
        .enum_values = {"celsius", "fahrenheit"},
        .name = "scale.temperature",
        .value.token_value = "fahrenheit",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .enum_values = {"12", "24"},
        .name = "time.format",
        .value.token_value = "12",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .enum_values = {"ERROR", "WARNING", "INFO", "DEBUG", "TRACE"},
        .name = "logs.level",
        .value.token_value = "INFO",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .name = "logs.color",
        .value.bool_value = false,
        .value_type = EZLOPI_SETTINGS_TYPE_BOOL
    },
    {
        .enum_values = {"-1", "2", "4", "8"},
        .name = "logs.indent",
        .value.token_value = "-1",
        .value_type = EZLOPI_SETTINGS_TYPE_TOKEN
    },
    {
        .name = "some_integer_value",
        .value.int_value = 42,
        .value_type = EZLOPI_SETTINGS_TYPE_INT
    }
};

// Function to modify a single setting value
void modifySetting(const char* name, const void* value) {
    for (size_t i = 0; i < sizeof(ezlopi_settings_list) / sizeof(ezlopi_settings_list[0]); i++) {
        if (strcmp(ezlopi_settings_list[i].name, name) == 0) {
            switch (ezlopi_settings_list[i].value_type) {
                case EZLOPI_SETTINGS_TYPE_TOKEN:
                    ezlopi_settings_list[i].value.token_value = (const char*)value;
                    break;
                case EZLOPI_SETTINGS_TYPE_BOOL:
                    ezlopi_settings_list[i].value.bool_value = *((bool*)value);
                    break;
                case EZLOPI_SETTINGS_TYPE_INT:
                    ezlopi_settings_list[i].value.int_value = *((int*)value);
                    break;
            }
            break;
        }
    }
}

const char *str_ezlopi_settings_date_format = "date.format";
const char *str_ezlopi_settings_time_format = "time.format";
const char *str_ezlopi_settings_first_start = "first_start";
// const char *str_ezlopi_settings_scale_temperature = "scale.temperature";

// Function to create a new setting
l_ezlopi_settings_t* ezlopi_settings_create_setting(const char* settings_name,
    void* settings_value,
    const void* settings_enum_values,
    uint16_t settings_enum_count,
    const char* settings_value_type,
    bool settings_is_enum)
{
    l_ezlopi_settings_t* setting = malloc(sizeof(l_ezlopi_settings_t));
    setting->settings_name = settings_name;
    setting->settings_value = settings_value;
    setting->settings_enum_values = settings_enum_values;
    setting->settings_enum_count = settings_enum_count;
    setting->settings_value_type = settings_value_type;
    setting->settings_is_enum = settings_is_enum;
    setting->next = NULL;
    return setting;
}

// Function to add a setting to the linked list
void ezlopi_settings_add_setting(l_ezlopi_settings_t** head, l_ezlopi_settings_t* setting) 
{
    if (*head == NULL) 
    {
        *head = setting;
    } 
    else 
    {
        l_ezlopi_settings_t* current = *head;
        while (current->next != NULL) 
        {
            current = current->next;
        }
        current->next = setting;
    }
}

void ezlopi_settings_modify_setting(l_ezlopi_settings_t* head, const char* setting_name, void* new_value) 
{

    l_ezlopi_settings_t* current = head;
    
    while (current != NULL)
    {
        if (strcmp(current->settings_name, setting_name) == 0) 
        {
            current->settings_value = new_value;
        }
        current = current->next;
    }
}

void ezlopi_settings_get_setting(l_ezlopi_settings_t* head, const char* setting_name, void ** get_value) 
{

    l_ezlopi_settings_t* current = head;
    
    while (current != NULL)
    {
        if (strcmp(current->settings_name, setting_name) == 0) 
        {
            *get_value = current->settings_value;
            return;         
        }
        current = current->next;
    }

    *get_value = NULL;
}

// Function to free the memory used by the linked list
void ezlopi_settings_free_settings(l_ezlopi_settings_t** head) 
{
    l_ezlopi_settings_t* current = *head;
    while (current != NULL) 
    {
        l_ezlopi_settings_t* next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

// Function to print the settings
void ezlopi_settings_print_settings(const l_ezlopi_settings_t* head) 
{
    TRACE_D("################################# Settings:\n ######################################## ");
    const l_ezlopi_settings_t* current = head;
    while (current != NULL) 
    {
        TRACE_D("Name: %s\n", current->settings_name);
        TRACE_D("Value: ");

        if (current->settings_is_enum) 
        {
            const char* enumValue = (const char*)current->settings_value;
            TRACE_D("%s\n", enumValue);
        } 
        else if (strcmp(current->settings_value_type, "bool") == 0) 
        {
            bool boolValue = *((bool*)current->settings_value);
            TRACE_D("%s\n", boolValue ? "true" : "false");
        }
        else if (strcmp(current->settings_value_type, "int") == 0) 
        {
            int intValue = *((int*)current->settings_value);
            TRACE_D("%d\n", intValue);
        } 
        else if (strcmp(current->settings_value_type, "float") == 0)
        {
            float floatValue = *((float*)current->settings_value);
            TRACE_D("%f\n", floatValue);
        } 
        else 
        {
            TRACE_D("Unknown settings_value type\n");
        }

        if (current->settings_is_enum) 
        {
            TRACE_D("Enum Values:\n");
            for (uint16_t j = 0; j < current->settings_enum_count; j++) 
            {
                const char* enumItem = ((const char**)current->settings_enum_values)[j];
                TRACE_D("%s\n", enumItem);
            }
        }
        current = current->next;
    }
}

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
