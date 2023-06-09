#include <string.h>
#include "stdint.h"
#include "settings.h"
#include "trace.h"

#include "cJSON.h"
#include "ezlopi_cloud_constants.h"
#include "ezlopi_cloud_methods_str.h"
#include "ezlopi_devices_list.h"
#include "web_provisioning.h"

void settings_list(cJSON *cj_request, cJSON *cj_response) {

    
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));

    cJSON *cj_result = cJSON_AddObjectToObject(cj_response, ezlopi_result);
    if (cj_result)
    {
        cJSON *cj_settings = cJSON_AddArrayToObject(cj_result, "settings");
        if (cj_settings)
        {

            // Retrieve the settings list
            s_ezlopi_settings_t *settings_list = ezlopi_settings_get_settings_list();

            // Calculate the number of settings
            size_t num_settings = ezlopi_settings_get_settings_count();

            // Add each setting to the "settings" array
            for (size_t i = 0; i < num_settings; i++) 
            {
                cJSON *cj_setting = cJSON_CreateObject();
                cJSON_AddItemToArray(cj_settings, cj_setting);

                // Add setting properties to the setting object
                cJSON_AddStringToObject(cj_setting, "name", settings_list[i].name);

                // Determine the value type and add the corresponding value
                switch (settings_list[i].value_type) 
                {
                    case EZLOPI_SETTINGS_TYPE_TOKEN:
                        cJSON_AddStringToObject(cj_setting, "value", settings_list[i].value.token_value);
                        cJSON_AddStringToObject(cj_setting, "valueType", "token");

                        // Create the "enum" array
                        cJSON *cj_enum = cJSON_CreateArray();
                        cJSON_AddItemToObject(cj_setting, "enum", cj_enum);

                        // Add enum values to the "enum" array
                        for (size_t j = 0; j < EZLOPI_SETTINGS_MAX_ENUM_VALUES; j++) {
                            const char *enum_value = settings_list[i].enum_values[j];
                            if (enum_value != NULL) {
                                cJSON_AddItemToArray(cj_enum, cJSON_CreateString(enum_value));
                            } else {
                                break;
                            }
                        }                        

                        break;
                    case EZLOPI_SETTINGS_TYPE_BOOL:
                        cJSON_AddBoolToObject(cj_setting, "value", settings_list[i].value.bool_value);
                        cJSON_AddStringToObject(cj_setting, "valueType", "bool");
                        break;
                    case EZLOPI_SETTINGS_TYPE_INT:
                        cJSON_AddNumberToObject(cj_setting, "value", settings_list[i].value.int_value);
                        cJSON_AddStringToObject(cj_setting, "valueType", "int");
                        break;
                    default:
                        break;
                }
            }
        }
    }
}
