#include <esp_system.h>
#include "ezlopi_cloud.h"

static uint64_t device_id = 0;
static uint64_t item_id = 0;
static uint64_t room_id = 0;
static uint64_t gateway_id = 0;
static uint64_t settings_id = 0;

static uint64_t ezlopi_get_mac(void)
{
    uint64_t mac_int = 0;
    uint8_t mac_base[6] = {0};
    esp_efuse_mac_get_default((uint8_t *)mac_base);
    
    for (int i = 0; i < 6; i++) 
    {
        mac_int = (mac_int << 8) | mac_base[i];
    }

    return mac_int;
}

uint64_t ezlopi_cloud_generate_device_id(void)
{
    if(0 == device_id)
    {
        device_id =  0x10000000 + ezlopi_get_mac();

    }
    // TRACE_D("device_id: %u\r\n", device_id);
    return device_id;
}

uint64_t ezlopi_cloud_generate_item_id(void)
{
    if(0 == item_id)
    {
        item_id =  0x20000000 + ezlopi_get_mac();

    }
    // TRACE_D("item_id: %u\r\n", item_id);

    return item_id;
}

uint64_t ezlopi_cloud_generate_room_id(void)
{
    if(0 == room_id)
    {
        room_id =  0x30000000 + ezlopi_get_mac();

    }
    // TRACE_D("room_id: %u\r\n", room_id);
    return room_id;
}

uint64_t ezlopi_cloud_generate_gateway_id(void)
{
    if(0 == gateway_id)
    {
        gateway_id =  0x40000000 + ezlopi_get_mac();

    }
    // TRACE_D("gateway_id: %u\r\n", gateway_id);
    return gateway_id;
}

uint64_t ezlopi_cloud_generate_settings_id(void)
{
    if(0 == settings_id)
    {
        settings_id =  0x50000000 + ezlopi_get_mac();

    }
    // TRACE_D("settings_id: %u\r\n", gateway_id);
    return settings_id;
}