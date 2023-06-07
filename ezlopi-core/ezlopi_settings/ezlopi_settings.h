#ifndef EZLOPI_SETTINGS_H
#define EZLOPI_SETTINGS_H

typedef enum {
    EZLOPI_SETTINGS_INITI_STATUS_TRUE,
    EZLOPI_SETTINGS_INITI_STATUS_FALSE
} e_ezlopi_settings_init_status_t;

// Structure to represent a single setting
typedef struct s_ezlopi_settings {
    const char* settings_name;
    void* settings_value;
    const void* settings_enum_values;
    uint16_t settings_enum_count;
    const char* settings_value_type;
    bool settings_is_enum;
    struct s_ezlopi_settings* next;
} l_ezlopi_settings_t;

void ezlopi_initialize_settings(void);

#endif  // EZLOPI_SETTINGS_H