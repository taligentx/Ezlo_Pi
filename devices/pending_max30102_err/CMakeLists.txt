file(GLOB_RECURSE config_src "*.c")
# set(config_src)

idf_component_register(SRCS "MAX30102_SPO2_algorithm.c" "sensor_0039_I2C_MAX30102.c"  "${config_src}"
                        INCLUDE_DIRS "."
                        REQUIRES trace ezlopi_actions ezlopi_hal ezlopi_timer ezlopi_devices ezlopi_cloud_constants gpio_isr_service ezlopi_cloud)