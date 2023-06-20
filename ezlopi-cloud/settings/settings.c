#include <string.h>
#include "stdint.h"
#include "settings.h"
#include "trace.h"

#include "cJSON.h"
#include "ezlopi_cloud_constants.h"
#include "ezlopi_cloud_methods_str.h"
#include "ezlopi_devices_list.h"
#include "web_provisioning.h"

void ezlopi_device_settings_list(cJSON *cj_request, cJSON *cj_response)
{
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));

    cJSON *cj_result = cJSON_AddObjectToObject(cj_response, ezlopi_result);
    if (cj_result)
    {
        cJSON *cj_settings_array = cJSON_AddArrayToObject(cj_result, "settings");
        if (cj_settings_array)
        {
            l_ezlopi_device_settings_t *registered_settings = ezlopi_devices_settings_get_list();

            ezlopi_device_settings_print_settings(registered_settings);

            while(NULL != registered_settings) 
            {
                if(NULL != registered_settings->properties)
                {
                    cJSON *cj_properties = cJSON_CreateObject();
                    if (cj_properties)
                    {

                        TRACE_E("Here");
                        char tmp_string[64];
                        snprintf(tmp_string, sizeof(tmp_string), "%08x", registered_settings->properties->id);
                        cJSON_AddStringToObject(cj_properties, "_id", tmp_string);
                        snprintf(tmp_string, sizeof(tmp_string), "%08x", registered_settings->properties->device_id);
                        cJSON_AddStringToObject(cj_properties, "deviceId", tmp_string);
                        cJSON_AddStringToObject(cj_properties, "label", registered_settings->properties->label);
                        cJSON_AddStringToObject(cj_properties, "description", registered_settings->properties->description);
                        cJSON_AddStringToObject(cj_properties, "status", "synced");
                        cJSON_AddStringToObject(cj_properties, "valueType", registered_settings->properties->value_type);
                        if(strcmp(registered_settings->properties->value_type, "action") == 0) 
                        {

                        }
                        else if(strcmp(registered_settings->properties->value_type, "bool") == 0)
                        {
                            if(registered_settings->properties->value.bool_value)
                            {
                                cJSON_AddTrueToObject(cj_properties, "value");
                            }
                            else 
                            {
                                cJSON_AddFalseToObject(cj_properties, "value");
                            }
                        }
                        else if(strcmp(registered_settings->properties->value_type, "int") == 0)
                        {
                            cJSON_AddNumberToObject(cj_properties, "value", registered_settings->properties->value.int_value);
                        }
                        else if(strcmp(registered_settings->properties->value_type, "string") == 0)
                        {
                            cJSON_AddStringToObject(cj_properties, "value", registered_settings->properties->value.string_value);
                        }
                        else if(strcmp(registered_settings->properties->value_type, "rgb") == 0)
                        {                            

                        }       
                        else if(strcmp(registered_settings->properties->value_type, "scalable") == 0)
                        {
                            cJSON *cj_value = cJSON_CreateObject();

                            if(cj_value)
                            {
                                cJSON_AddStringToObject(cj_value, "scale", registered_settings->properties->value.scalable_value->scale);
                                cJSON_AddNumberToObject(cj_value, "value", registered_settings->properties->value.scalable_value->value);
                                cJSON_AddItemToObject(cj_properties, "value", cj_value);
                            }                            
                        }
                        else 
                        {

                        }    

                        if (!cJSON_AddItemToArray(cj_settings_array, cj_properties))
                        {
                            cJSON_Delete(cj_properties);
                        }                                                                                       
                    }
                }
                registered_settings = registered_settings->next;
            }
        }
    }    
}

void ezlopi_device_settings_value_set(cJSON *cj_request, cJSON *cj_response)
{
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));
    cJSON_AddObjectToObject(cj_response, ezlopi_result);

    cJSON *cj_params = cJSON_GetObjectItem(cj_request, "params");
    if (cj_params)
    {
        char *settings_id_str = 0;
        CJSON_GET_VALUE_STRING(cj_params, ezlopi__id_str, settings_id_str);
        int settings_id = strtol(settings_id_str, NULL, 16);
        TRACE_I("settings_id_str: %X", settings_id);

        l_ezlopi_device_settings_t *registered_settings = ezlopi_devices_settings_get_list();
        while (NULL != registered_settings)
        {
            if (registered_settings->properties)
            {
                if (settings_id == registered_settings->properties->id)
                {
                   _ezlopi_device_settings_value_set(settings_id, cj_params);
                }
            }

            registered_settings = registered_settings->next;
        }
    }
}

void ezlopi_device_settings_reset(cJSON *cj_request, cJSON *cj_response)
{
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));
    cJSON_AddObjectToObject(cj_response, ezlopi_result);

    cJSON *cj_params = cJSON_GetObjectItem(cj_request, "params");
    if (cj_params)
    {


        l_ezlopi_device_settings_t *registered_settings = ezlopi_devices_settings_get_list();
        while (NULL != registered_settings)
        {
            if (registered_settings->properties)
            {

                if (cJSON_HasObjectItem(cj_params, ezlopi__id_str)) 
                {
                    
                    char *settings_id_str = 0;
                    CJSON_GET_VALUE_STRING(cj_params, ezlopi__id_str, settings_id_str);
                    int settings_id = strtol(settings_id_str, NULL, 16);
                    TRACE_I("settings_id_str: %X", settings_id);   

                    if (settings_id == registered_settings->properties->id)
                    {                           
                        _ezlopi_device_settings_reset_settings_id(settings_id);
                    }                    

                }
                else if(cJSON_HasObjectItem(cj_params, ezlopi_deviceId_str))
                {
                    char *device_id_str = 0;
                    CJSON_GET_VALUE_STRING(cj_params, ezlopi_deviceId_str, device_id_str);
                    int device_id = strtol(device_id_str, NULL, 16);
                    TRACE_I("device_id_str: %X", device_id); 

                    if (device_id == registered_settings->properties->device_id)
                    {                        
                        _ezlopi_device_settings_reset_device_id(device_id);   
                    }             

                }
            }

            registered_settings = registered_settings->next;
        }
    }
}


void ezlopi_settings_list(cJSON *cj_request, cJSON *cj_response) {

    
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));

    cJSON *cj_result = cJSON_AddObjectToObject(cj_response, ezlopi_result);
    if (cj_result)
    {
        cJSON *cj_settings = cJSON_AddArrayToObject(cj_result, "settings");
        if (cj_settings)
        {

            // Retrieve the settings list
            s_ezlopi_hub_settings_t *settings_list = ezlopi_settings_get_settings_list();

            // Calculate the number of settings
            uint16_t num_settings = ezlopi_settings_get_settings_count();

            // Add each setting to the "settings" array
            for (uint16_t i = 0; i < num_settings; i++) 
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
                        for (uint16_t j = 0; j < EZLOPI_SETTINGS_MAX_ENUM_VALUES; j++) {
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

void ezlopi_settings_value_set(cJSON *cj_request, cJSON *cj_response)
{
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));

    cJSON *cj_params = cJSON_GetObjectItem(cj_request, "params");
    if (cj_params)
    {
        char *settings_name = NULL;
        
        CJSON_GET_VALUE_STRING(cj_params, ezlopi_name_str, settings_name);
        cJSON* key_settings_value = cJSON_GetObjectItem(cj_params, ezlopi_value_str);
        
        if (cJSON_IsBool(key_settings_value)) 
        {
            bool settings_val_bool;
            cJSON_bool _settings_val_bool = cJSON_IsTrue(key_settings_value);
            settings_val_bool = _settings_val_bool ? true : false;
            ezlopi_settings_modify_setting(settings_name, &settings_val_bool);
                       
        }
        else if(cJSON_IsNumber(key_settings_value))
        {
            int settings_val_int = (int)cJSON_GetNumberValue(key_settings_value);
            ezlopi_settings_modify_setting(settings_name, &settings_val_int);
        }
        else if(cJSON_IsString(key_settings_value))
        {
            char *settings_val_str = cJSON_GetStringValue(key_settings_value);
            TRACE_W("settings_val_str: %s", settings_val_str);
            ezlopi_settings_modify_setting(settings_name, settings_val_str);
        }
        else 
        {

        }        
    }
}

void ezlopi_settings_value_set_response(cJSON *cj_request, cJSON *cj_response)
{
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_id_str, cJSON_GetObjectItem(cj_request, ezlopi_id_str));
    cJSON_AddItemReferenceToObject(cj_response, ezlopi_key_method_str, cJSON_GetObjectItem(cj_request, ezlopi_key_method_str));

    cJSON_AddNullToObject(cj_response, ezlopi_str_error);

    cJSON_AddObjectToObject(cj_response, ezlopi_result);   
}