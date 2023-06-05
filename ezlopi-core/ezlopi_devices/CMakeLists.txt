# Set source files
# set(config_src "config.cpp")
file(GLOB_RECURSE config_src "*.c")

# Register component
idf_component_register(
    SRCS
        "ezlopi_devices.c"
        "ezlopi_device_value_updated.c"
        "ezlopi_devices_list.c"
        "${config_src}"
    INCLUDE_DIRS
        "."
    REQUIRES
        trace
        json
        ezlopi_nvs
        ezlopi_hal
        ezlopi_cloud
        device_0001_digitalOut_generic
        device_0002_digitalOut_relay
        device_0003_digitalOut_plug
        sensor_0019_digitalIn_PIR
        sensor_0018_internal_hall_effect
        device_0022_PWM_dimmable_lamp
        sensor_0020_ADC_2axis_joystick
        sensor_0024_other_HCSR04
        sensor_0023_digitalIn_touch_switch_TTP223B
        sensor_0021_UART_MB1013
        device_0036_PWM_servo_MG996R
        sensor_0025_digitalIn_LDR
        sensor_0026_ADC_LDR
        sensor_0027_ADC_waterLeak    
        sensor_0029_I2C_GXHTC3
        sensor_0030_oneWire_DS18B20
        sensor_0032_ADC_soilMoisture
        sensor_0016_OneWire_DHT22
        sensor_0031_other_JSNSR04T
        sensor_0033_ADC_turbidity
        sensor_0034_digitalIn_proximity
        sensor_0035_digitalIn_touch_sensor_TPP223B
        sensor_0017_ADC_potentiometer
        sensor_0005_I2C_MPU6050
        sensor_0006_I2C_ADXL345
)


