#ifndef _0007_SENS_I2C_MAGNETOMETER_H_
#define _0007_SENS_I2C_MAGNETOMETER_H_

#include "ezlopi_actions.h"
#include "ezlopi_i2c_master.h"
#include "ezlopi_devices.h"

#define GY271_ADDR 0x0D
/*******************************************************************/
// REGISTER ADDRESS
/*******************************************************************/
//
// 1. OUTPUT DATA REGISTERS . 16-bit  -> 2^16 = [-32768 ; +32767]
#define GY271_DATA_X_LSB_REGISTER 0x00 // [R] [LSB] This is read-only register.  // little endian
#define GY271_DATA_X_MSB_REGISTER 0x01 // [R] [MSB] This is read-only register.  // MSB contains (+-sign)
#define GY271_DATA_Y_LSB_REGISTER 0x02 // [R] [LSB] This is read-only register.  //
#define GY271_DATA_Y_MSB_REGISTER 0x03 // [R] [MSB] This is read-only register.
#define GY271_DATA_Z_LSB_REGISTER 0x04 // [R] [LSB] This is read-only register.
#define GY271_DATA_Z_MSB_REGISTER 0x05 // [R] [MSB] This is read-only register.

#define GY271_STATUS_REGISTER 0x06 // [R] This is read-only register.

#define GY271_DATA_TEMP_LSB_REGISTER 0x07 // [R] This is read-only register.  // sensitivity 100 LSB/c
#define GY271_DATA_TEMP_MSB_REGISTER 0x08

#define GY271_CONTROL_REGISTER_1 0x09 // [R+W]  // MODE ; ODR ; RNG ; OSR
#define GY271_CONTROL_REGISTER_2 0x0A // [R+W]  // Soft-reset ; roll-over pointer ; INT_EN

#define GY271_SET_RESET_PERIOD_REGISTER 0x0B // [R+W]   // 8-bit reset period value

/*******************************************************************/
// Configuration values
/*******************************************************************/
// Mode Control (MODE)
#define GY271_MODE_STDBY (0x00)
#define GY271_MODE_CONT (1 << 0)
// Output Data Rate (ODR)
#define GY271_ODR_10hz (0x00)
#define GY271_ODR_50hz (1 << 2)
#define GY271_ODR_100hz (1 << 3)
#define GY271_ODR_200hz (1 << 3) | (1 << 2)
// Full Scale Range (RNG)
#define GY271_RNG_2G (0x00)
#define GY271_RNG_8G (1 << 4)
// Over Sample Ratio (OSR)
#define GY271_OSR_512 (0x00)
#define GY271_OSR_256 (1 << 6)
#define GY271_OSR_128 (1 << 7)
#define GY271_OSR_64 (1 << 7) | (1 << 6)

// Define Set-Reset period [in 0x0B] + Masked Bits.
// 00-> For StandBy mode ; 01-> ForContinous Mode
#define GY271_DEFAULT_SET_RESET_PERIOD (0x01)
// Data format for control register1 [in 0x09] + Masked Bits.
//  e.g:-
//  OSR  = 01 [256]
//  RNG  = 00 [+-2G]    with 12000 LSB/G
//  ODR  = 00 [10Hz]
//  MODE = 01 [continous]
#define GY271_OPERATION_MODE (GY271_OSR_128) | (GY271_RNG_2G) | (GY271_ODR_50hz) | (GY271_MODE_CONT)

// Enable INTERUPT bit from control register2 [in 0x0A] + Masked Bits.
#define GY271_INT_EN (0x00)

// Data ready flag [Masked Bit0] in "{STATUS_REGISTER - 06H}"
#define GY271_DATA_READY_FLAG (1 << 0)

// Data Skip flag [Masked Bit2] in "{STATUS_REGISTER - 06H}"
#define GY271_DATA_SKIP_FLAG (1 << 2)

/*******************************************************************/
// list of cosntants
/*******************************************************************/
// Offsets and conversion values : 1g = 9.80665 m/s^2
#define GY271_STANDARD_G_TO_ACCEL_CONVERSION_VALUE (9.80665f)
// Following are the conversion factor of data according to the datahsheet.
#define GY271_TEMPERATURE_SENSITIVITY 100.0f // [100 LSB/*c]
// At Field_Range = +-2G (), the conversion LSB per G => 12000 LSB/Gauss.
#define GY271_CONVERSION_TO_G 12000.0f

// Pi
#define PI (3.1416f)

// CUSTOM GY271 Data storage structure
typedef struct gy271_raw_data_tt
{
    int16_t raw_x;
    int16_t raw_y;
    int16_t raw_z;
    int16_t raw_temp;
} gy271_raw_data_t;

typedef struct gy271_data_tt
{
    float X;
    float Y;
    float Z;
    float T;
    int azimuth;
} gy271_data_t;

// Action
int sensor_0007_I2C_GY271(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg);

#endif // _0007_SENS_I2C_MAGNETOMETER_H_