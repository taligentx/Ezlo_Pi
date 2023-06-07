#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "ezlopi_settings.h"
#include "ezlopi_nvs.h"

#define MAX_ENUM_VALUES 10

const char *str_ezlopi_settings_date_format = "date.format";
const char *str_ezlopi_settings_time_format = "time.format";
const char *str_ezlopi_settings_first_start = "first_start";
const char *str_ezlopi_settings_scale_temperature = "scale.temperature";

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
void ezlopi_settings_add_setting(l_ezlopi_settings_t** head, l_ezlopi_settings_t* setting) {
    if (*head == NULL) {
        *head = setting;
    } else {
        l_ezlopi_settings_t* current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = setting;
    }
}

// Function to free the memory used by the linked list
void ezlopi_settings_free_settings(l_ezlopi_settings_t** head) {
    l_ezlopi_settings_t* current = *head;
    while (current != NULL) {
        l_ezlopi_settings_t* next = current->next;
        free(current);
        current = next;
    }
    *head = NULL;
}

// Function to print the settings
void ezlopi_settings_print_settings(const l_ezlopi_settings_t* head) {
    printf("Settings:\n");
    const l_ezlopi_settings_t* current = head;
    while (current != NULL) {
        printf("Name: %s\n", current->settings_name);
        printf("Value: ");

        if (current->settings_is_enum) {
            const char* enumValue = (const char*)current->settings_value;
            printf("%s\n", enumValue);
        } else if (strcmp(current->settings_value_type, "bool") == 0) {
            bool boolValue = *((bool*)current->settings_value);
            printf("%s\n", boolValue ? "true" : "false");
        } else if (strcmp(current->settings_value_type, "int") == 0) {
            int intValue = *((int*)current->settings_value);
            printf("%d\n", intValue);
        } else if (strcmp(current->settings_value_type, "float") == 0) {
            float floatValue = *((float*)current->settings_value);
            printf("%f\n", floatValue);
        } else {
            printf("Unknown settings_value type\n");
        }

        if (current->settings_is_enum) {
            printf("Enum Values:\n");
            for (uint16_t j = 0; j < current->settings_enum_count; j++) {
                const char* enumItem = ((const char**)current->settings_enum_values)[j];
                printf("%s\n", enumItem);
            }
        }

        current = current->next;
    }
}

void ezlopi_initialize_settings(void) {


    // Create settings linked list
    l_ezlopi_settings_t* settings_list = NULL;

    const char* ezlopi_settings_enum_value_date_format[] = { "mmddyy", "ddmmyy" };
    const uint16_t ezlopi_settings_enum_count_date_format = sizeof(ezlopi_settings_enum_value_date_format) / sizeof(ezlopi_settings_enum_value_date_format[0]);
    const char* ezlopi_settings_enum_value_date_format_default = ezlopi_settings_enum_value_date_format[0];
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_date_format, (void*)ezlopi_settings_enum_value_date_format_default, ezlopi_settings_enum_value_date_format, ezlopi_settings_enum_count_date_format, "token", true));

    bool firstStart = true;
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_first_start, (void*)&firstStart, NULL, 0, "bool", false));

    const char* ezlopi_settings_enum_scale_temperature[] = { "celsius", "fahrenheit" };
    const uint16_t ezlopi_settings_enum_count_scale_temperature = sizeof(ezlopi_settings_enum_scale_temperature) / sizeof(ezlopi_settings_enum_scale_temperature[0]);
    const char* ezlopi_settings_enum_scale_temperature_default = ezlopi_settings_enum_scale_temperature[1];
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_scale_temperature, (void*)ezlopi_settings_enum_scale_temperature_default, ezlopi_settings_enum_scale_temperature, ezlopi_settings_enum_count_scale_temperature, "token", true));

    const char* ezlopi_settings_enum_time_format[] = { "12", "24" };
    const uint16_t ezlopi_settings_enum_count_time_format = sizeof(ezlopi_settings_enum_time_format) / sizeof(ezlopi_settings_enum_time_format[0]);
    const char* ezlopi_settings_enum_time_format_default = ezlopi_settings_enum_time_format[0];
    ezlopi_settings_add_setting(&settings_list, ezlopi_settings_create_setting(str_ezlopi_settings_time_format, (void*)ezlopi_settings_enum_time_format_default, ezlopi_settings_enum_time_format, ezlopi_settings_enum_count_time_format, "token", true));

    
    // if(EZLOPI_SETTINGS_INITI_STATUS_FALSE == ezlopi_nvs_get_settings_init_status())
    {
        ezlopi_settings_save_settings(settings_list);

        ezlopi_nvs_set_settings_init_status();
    }
        
    if(EZLOPI_SETTINGS_INITI_STATUS_TRUE == ezlopi_nvs_get_settings_init_status()) 
    {
        // ezlopi_settings_retrive_settings();
    }

    // Print settings
    ezlopi_settings_print_settings(settings_list);

    // Clean up
    ezlopi_settings_free_settings(&settings_list);

    return 0;
}
