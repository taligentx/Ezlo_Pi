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


#define EZLOPI_SETTINGS_MAX_ENUM_VALUES 10

typedef enum {
    EZLOPI_SETTINGS_TYPE_TOKEN,
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

#endif  // EZLOPI_SETTINGS_H