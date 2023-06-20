#ifndef _sensor_0039_I2C_MAX30102_H_
#define _sensor_0039_I2C_MAX30102_H_

#include "ezlopi_actions.h"
#include "ezlopi_devices.h"

#define MAX30102_LED_COUNT 2

#define MAX30102_SLAVEID (0x57)
#define MAX30102_SENSE_BUF_SIZE 30 // since 6*5 = 30 ; 2 is left out
//-------------------------
// #REGISTER ADDRESSES
//-------------------------
// Status Registers [R]
#define MAX30102_INTSTAT1 0x00   // A_FULL ; PPG_RDY ; ALC_OVF ; PWR_RDY
#define MAX30102_INTSTAT2 0x01   // DIE_TEMP_RDY
#define MAX30102_INTENABLE1 0x02 // A_FULL_EN ; PPG_RDY_EN ; ALC_OVF_EN
#define MAX30102_INTENABLE2 0x03 // DIE_TEMP_RDY_EN

// FIFO Registers [R+W]
#define MAX30102_FIFO_WRITEPTR 0x04
#define MAX30102_FIFO_OVERFLOW 0x05
#define MAX30102_FIFO_READPTR 0x06
#define MAX30102_FIFO_DATA 0x07

// Configuration Registers [R+W]
#define MAX30102_FIFOCONFIG 0x08
#define MAX30102_MODECONFIG 0x09
#define MAX30102_SPO2CONFIG 0x0A // Note, sometimes listed as "PARTICLE" config in datasheet (pg. 11)
#define MAX30102_LED1_PULSEAMP 0x0C
#define MAX30102_LED2_PULSEAMP 0x0D
#define MAX30102_MULTILEDCONFIG1 0x11
#define MAX30102_MULTILEDCONFIG2 0x12

// Die Temperature Registers [R]
#define MAX30102_DIETEMPINT 0x1F
#define MAX30102_DIETEMPFRAC 0x20
#define MAX30102_DIETEMPCONFIG 0x21

// Rev/Part ID Registers address [R]
#define MAX30102_REVISIONID 0xFE
#define MAX30102_PARTID 0xFF

// Should always be 0x15 for MAX30101 ; MAX30102.
#define MAX30102_EXPECTEDPARTID (0x15) // check this value in PART_ID register
//-----------------------------------
// #Configuration Mode + mode Masks [R+W]
//------------------------------------

// [R] Interrupt condition for [INTSTATUS1 (0x00H) & INTSTATUS2 (0x01H)] to compare
#define FIFO_A_FULL (1 << 7)  // Indicates limited storage spaces are left
                              // we can set the limits using "FIFO_A_FULL" register [in 0x08H]
                              // this flag is cleared by reading 0x00H
#define PPG_READY (1 << 6)    // Indicates new sample being added in data FIFO
#define ALC_OVF (1 << 5)      // Indicates the ambient interference in output results
#define PWR_READY (1 << 0)    // Indicates the system is ready to operate stably
                              // Is high during MAX30102 is ON
#define DIE_TEMP_RDY (1 << 1) // Indicates Ready when internal die temperature conversion is finished,
//---------

//---------
// [W] Configuration values to write the [INTEN1 (0x02H) & INTEN2 (0x03H)] registers
#define FIFO_A_FULL_EN (1 << 7)  // allow "FIFO_A_FULL" flag in 0x00H
#define PPG_READY_EN (1 << 6)    // allow "PPG_READY" flag in 0x00H
#define ALC_OVF_EN (1 << 5)      // allow "ALC_OVF" flag in 0x00H
#define DIE_TEMP_RDY_EN (1 << 1) // allow "DIE_TEMP_RDY" flag in 0x01H

#define DIE_TEMP_GAURD (1 << 0) // Activate/Use when needed // [W] Before you read the temperature values from [(0x1F) & (0x20)] ; Set this Gaurd flag;
                                // Auto-Reset to zero, after the temperature data is read from [(0x1F) & (0x20)].
                                // Note: Read temperature :- 1) enable "DIE_TEMP_RDY_EN" bit. 2) Check "DIE_TEMP_RDY" flag. 3) enable "DIE_TEMP_GAURD". 4) Read "TINTEGER (0x1F) + TFRACTION (0x20)"
//---------

//---------
// [W] FIFO Configuration
// 1. SMP_AVG [7:5]
#define MAX30102_FIFO_SAMPLEAVG_1 (0x00) // default : no averaging
#define MAX30102_FIFO_SAMPLEAVG_2 (1 << 5)
#define MAX30102_FIFO_SAMPLEAVG_4 (1 << 6)
#define MAX30102_FIFO_SAMPLEAVG_8 (1 << 5) | (1 << 6)
#define MAX30102_FIFO_SAMPLEAVG_16 (1 << 7)
#define MAX30102_FIFO_SAMPLEAVG_32 (1 << 7) | (1 << 5)
// 2. FIFO_rollover [4th]
#define FIFO_ROLLOVER_DISABLE (0 << 4) // [W] 0-> when the FIFO becomes completely filled with data; Does not overwrite , unless FIFO_DATA is READ or FIFO_READ_WRITE_PTR are changed.
#define FIFO_ROLLOVER_ENABLE (1 << 4)  // [W] 1-> when the FIFO becomes completely filled with data; Automatically overwrites the old data with new data.
// 3. FIF0_A_FULL interrupt limit [3:0]
#define FIFO_FULL_INTERRUPT_AT_0_SPACE_LEFT (0x00)   // max //ie. when all 32-FIFO space are occupied. So 0-space left to store ; [Here: 1-space => 1-sample => 6bytes]
#define FIFO_FULL_INTERRUPT_AT_2_SPACE_LEFT (1 << 1) // max //ie. when all 30-FIFO space are occupied. So 2-space left to store ; [Here: 1-space => 1-sample => 6bytes]
#define FIFO_FULL_INTERRUPT_AT_17_SPACE_LEFT (0x0F)  // min //ie. when all 17-FIFO space are occupied. So 15-space left to store
//---------

//---------
// [W] Mode Configuration
// 1. Shutdown
#define MAX30102_SHTDWN_MODE (1 << 7) // default 0 // All interrupts are cleared to zero in this mode
// 2. Reset
#define MAX30102_RESET (1 << 6) // Auto cleared // When set-1 :all configuration, threshold, and data registers are reset to their power-on-state
                                // Note: Setting the RESET bit does not trigger a PWR_RDY interrupt event.
// 3. Operational Mode [2:0]
#define MAX30102_HB_MODE (1 << 1)                         // Heart_rate mode
#define MAX30102_SPO2_MODE (1 << 1) | (1 << 0)            // SPO2 mode
#define MAX30102_MLED_MODE (1 << 2) | (1 << 1) | (1 << 0) // Multi-LED mode
//---------

//---------
// [W] SPO2 Configuration
// 1. SPO2_ADC_RNG(18-bit resolution) [6:5]
#define MAX30102_SPO2_ADC_FS_2048 (0x00)               // 7.81 LSB (1-bit = 7.81 pA)   //+-2048 nA
#define MAX30102_SPO2_ADC_FS_4096 (1 << 5)             // 15.63 LSB (1-bit = 15.63 pA) //+-4096 nA
#define MAX30102_SPO2_ADC_FS_8192 (1 << 6)             // 31.25 LSB (1-bit = 31.25 pA) //+-8192 nA
#define MAX30102_SPO2_ADC_FS_16384 (1 << 6) | (1 << 5) // 62.5 LSB (1-bit = 62.5 pA)   //+-16384 nA
// 2. SPO2_SR [4:2] // 1-sample = [1 IR_pulse/conv] + [1 Red_pulse/conv]
#define MAX30102_SPO2_SR_50 (0x00)                           // 50 samples/sec
#define MAX30102_SPO2_SR_100 (1 << 2)                        // 100 samples/sec  //default
#define MAX30102_SPO2_SR_200 (1 << 3)                        // 200 sample/sec
#define MAX30102_SPO2_SR_400 (1 << 3) | (1 << 2)             // 400 sample/sec
#define MAX30102_SPO2_SR_800 (1 << 4)                        // 800 sample/sec
#define MAX30102_SPO2_SR_1000 (1 << 4) | (1 << 2)            // sample/sec
#define MAX30102_SPO2_SR_1600 (1 << 4) | (1 << 3)            // sample/sec
#define MAX30102_SPO2_SR_3200 (1 << 4) | (1 << 3) | (1 << 2) // sample/sec
// 3. LED_PulseWidth [1:0]
#define MAX30102_SPO2_LedPulseWidth_15 (0x00)              // 68.5uS  // 15-bit  resolution //
#define MAX30102_SPO2_LedPulseWidth_16 (1 << 0)            // 118uS   // 16-bit resolution //
#define MAX30102_SPO2_LedPulseWidth_17 (1 << 1)            // 215uS   // 17-bit resolution //
#define MAX30102_SPO2_LedPulseWidth_18 (1 << 1) | (1 << 0) // 411uS   // 18-bit resolution //
//---------

//---------
// [W] LED Pulse_Amp and Current_control
// (0x0C) 1. LED1-RED_PA [7:0] ; AMP => 6.2mA
#define MAX30102_RED_LED1_PA_6_2mA (0x1F) // 0b00011111 // Set RED-led (LED1) amp to consume 6.2mA current
// (0x0D) 2. LED2-IR_PA [7:0]  ; AMP => 6.2mA
#define MAX30102_IR_LED2_PA_6_2mA (0x1F) // 0b00011111 // Set IR-led (LED2) amp to consume 6.2mA current
//---------

//---------
// [W] Multi-LED Mode control (decide which led to activate in various time slots)
// (0x11) 1. SLOT1 [2:0]  -> activate RED_led
#define MAX30102_SLOT1_RED (1 << 0) // Choosing RED with Amp of [LED1-RED_PA]
// (0x11) 2. SLOT2 [6:4]  -> activate IR_led
#define MAX30102_SLOT2_IR (1 << 5) // Choosing IR with Amp of [LED2-IR_PA]
//---------

/* Custom data_structure to store HP & SPO2 values */
typedef struct
{
    uint32_t red[MAX30102_SENSE_BUF_SIZE];
    uint32_t IR[MAX30102_SENSE_BUF_SIZE];
    uint8_t head;
    uint8_t tail;
} sSenseBuf_t;
//-------------- ACTION DECLARATION ----------------------
int sensor_0039_I2C_MAX30102(e_ezlopi_actions_t action, s_ezlopi_device_properties_t *properties, void *arg, void *user_arg);
//------------------------------------
#endif //_sensor_0039_I2C_MAX30102_H_