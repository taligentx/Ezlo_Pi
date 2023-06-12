#ifndef EZLOPI_SETTINGS_H
#define EZLOPI_SETTINGS_H

#include <stdbool.h>

#define     EZLOPI_SETTINGS_MAGIC_NUMBER    (uint32_t)0x457A5069

#define EZLOPI_SETTINGS_MAX_ENUM_VALUES 10

typedef enum {
    EZLOPI_SETTINGS_TYPE_TOKEN = 0,
    EZLOPI_SETTINGS_TYPE_BOOL,
    EZLOPI_SETTINGS_TYPE_INT,
    EZLOPI_SETTINGS_TYPE_MAX
} e_ezlopi_settings_value_type_t;

typedef struct s_ezlopi_settings {
    const char* enum_values[EZLOPI_SETTINGS_MAX_ENUM_VALUES];
    const char* name;
    union {
        const char* token_value;
        bool bool_value;
        int int_value;
    } value;
    e_ezlopi_settings_value_type_t value_type;
} s_ezlopi_settings_t;

void ezlopi_initialize_settings(void);
uint16_t ezlopi_settings_get_settings_count(void);
s_ezlopi_settings_t *ezlopi_settings_get_settings_list(void);

#endif  // EZLOPI_SETTINGS_H