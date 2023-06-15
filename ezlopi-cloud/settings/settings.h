#ifndef __HUB_SETTINGS_H__
#define __HUB_SETTINGS_H__

#include <string.h>
#include "cJSON.h"
#include "ezlopi_settings.h"

#ifdef __cplusplus
extern "C"
{
#endif
    void ezlopi_device_settings_list(cJSON *cj_request, cJSON *cj_response);
    void ezlopi_device_settings_value_set(cJSON *cj_request, cJSON *cj_response);
    void ezlopi_device_settings_reset(cJSON *cj_request, cJSON *cj_response);


    void settings_list(cJSON *cj_request, cJSON *cj_response);
    void settings_value_set(cJSON *cj_request, cJSON *cj_response);
    void settings_value_set_response(cJSON *cj_request, cJSON *cj_response);
    void settings_updated(cJSON *cj_request, cJSON *cj_response);

#ifdef __cplusplus
}
#endif

#endif // __HUB_SETTINGS_H__