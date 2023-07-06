#ifndef dscKeybus_h
#define dscKeybus_h

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "driver/gpio.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "debug.h"
#include "driver/timer.h"

// Configures the Keybus interface with the specified GPIO pins
#if defined(CONFIG_IDF_TARGET_ESP32)
#define dscClockPin 18
#define dscReadPin  19
#define dscWritePin 21

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define dscClockPin 14
#define dscReadPin  15
#define dscWritePin 16
#endif

// dscKeybus public
void dsc_init(void);
void dscStop();           // Disables the clock hardware interrupt and data timer interrupt
void dscResetStatus();    // Resets the state of all status components as changed for sketches to get the current status

// Data setup - partitions, zones, and buffer size can be customized
#define dscPartitions 8   // Maximum number of partitions - requires 19 bytes of memory per partition
#define dscZones 8        // Maximum number of zone groups, 8 zones per group - requires 6 bytes of memory per zone group
#define dscBufferSize 50  // Number of commands to buffer if the sketch is busy - requires dscReadSize + 2 bytes of memory per command
#define dscReadSize 16    // Maximum bytes of a Keybus command

// Write
void dscWriteKey(int receivedKey);             // Writes a single key
void dscWriteKeys(const char * receivedKeys);  // Writes multiple keys from a char array
uint8_t dscWritePartition;                     // Set to a partition number for virtual keypad

// Panel time
bool dscTimestampChanged;          // True after the panel sends a timestamped message
uint8_t dscHour, dscMinute, dscDay, dscMonth;
int dscYear;

// Sets panel time, the dscYear can be sent as either 2 or 4 digits
void dscSetTime(unsigned int dscYear, uint8_t dscMonth, uint8_t dscDay, uint8_t dscHour, uint8_t dscMinute, const char* dscAccessCode);

// Status tracking
bool dscStatusChanged;                      // True after any status change
bool dscPauseStatus;                        // Prevent status from showing as changed, set in sketch to control when to update status
bool dscKeybusConnected, dscKeybusChanged;  // True if data is detected on the Keybus
uint8_t dscAccessCode[dscPartitions];
bool dscAccessCodeChanged[dscPartitions];
bool dscAccessCodePrompt;                   // True if the panel is requesting an access code
bool dscTrouble, dscTroubleChanged;
bool dscPowerTrouble, dscPowerChanged;
bool dscBatteryTrouble, dscBatteryChanged;
bool dscKeypadFireAlarm, dscKeypadAuxAlarm, dscKeypadPanicAlarm;
bool dscReady[dscPartitions], dscReadyChanged[dscPartitions];
bool dscArmed[dscPartitions], dscArmedAway[dscPartitions], dscArmedStay[dscPartitions];
bool dscNoEntryDelay[dscPartitions], dscArmedChanged[dscPartitions];
bool dscAlarm[dscPartitions], dscAlarmChanged[dscPartitions];
bool dscExitDelay[dscPartitions], dscExitDelayChanged[dscPartitions];
uint8_t dscExitState[dscPartitions], dscExitStateChanged[dscPartitions];
bool dscEntryDelay[dscPartitions], dscEntryDelayChanged[dscPartitions];
bool dscFire[dscPartitions], dscFireChanged[dscPartitions];
bool dscOpenZonesStatusChanged;
uint8_t dscOpenZones[dscZones], dscOpenZonesChanged[dscZones];    // Zone status is stored in an array using 1 bit per zone, up to 64 zones
bool dscAlarmZonesStatusChanged;
uint8_t dscAlarmZones[dscZones], dscAlarmZonesChanged[dscZones];  // Zone alarm status is stored in an array using 1 bit per zone, up to 64 zones

// dscPanelData[] stores panel data in an array: command [0], stop bit by itself [1], followed by the
// remaining data.  These can be accessed directly in the program to get data that is not already
// tracked in the library.  See dscKeybusProtocol.c for the currently known DSC commands and data.
//
// dscPanelData[] example:
//   Byte 0     Byte 2   Byte 3   Byte 4   Byte 5
//   00000101 0 10000001 00000001 10010001 11000111 [0x05] Status lights: Ready Backlight | Partition ready
//            ^ Byte 1 (stop bit)
uint8_t dscPanelData[dscReadSize];

// dscStatus[] and dscLights[] store the current status message and LED state for each partition.  These can be accessed
// directly in the sketch to get data that is not already tracked in the library.  See dscPrintPanelMessages() and
// dscPrintPanelLights() in dscKeybusProtocol.c to see how this data translates to the status message and LED status.
uint8_t dscStatus[dscPartitions];
uint8_t dscLights[dscPartitions];

volatile bool dscBufferOverflow;

// dscKeybus private

// Process panel data
void dscPanelLoop();
void dscLoop();

// Task handling
TaskHandle_t dscPanelLoopHandle;
SemaphoreHandle_t dscDataAvailable;
bool dscPanelDataAvailable;

#define dscClockPinMask (1ULL << dscClockPin)
#define dscReadPinMask  (1ULL << dscReadPin)
#define dscWritePinMask (1LL << dscWritePin)

// Timer interrupt setup
#define dscTimerGroup TIMER_GROUP_0
#define dscTimerNumber TIMER_0
#define dscTimerInterval 50
#define dscTimerDivider 80
#define dscTimerISRPriority (ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM)
#define dscGPIOISRPriority (ESP_INTR_FLAG_LEVEL2 | ESP_INTR_FLAG_IRAM)

// Exit delay target states
#define DSC_EXIT_STAY 1
#define DSC_EXIT_AWAY 2
#define DSC_EXIT_NO_ENTRY_DELAY 3

// Syntax compatibility wrappers
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

void dscProcessPanelStatus();
void dscProcessPanelStatus0(uint8_t partition, uint8_t dscPanelByte);
void dscProcessPanelStatus2(uint8_t partition, uint8_t dscPanelByte);
void dscProcessPanelStatus4(uint8_t partition, uint8_t dscPanelByte);
void dscProcessPanel_0x27();
void dscProcessPanel_0x2D();
void dscProcessPanel_0x34();
void dscProcessPanel_0x3E();
void dscProcessPanel_0xA5();
void dscProcessPanel_0xE6();
void dscProcessPanel_0xE6_0x09();
void dscProcessPanel_0xE6_0x0B();
void dscProcessPanel_0xE6_0x0D();
void dscProcessPanel_0xE6_0x0F();
void dscProcessPanel_0xEB();

void dscSetup();
bool dscValidCRC();
void dscSetWriteKey(int receivedKey);
bool dscRedundantPanelData(uint8_t dscPreviousCmd[], volatile uint8_t dscCurrentCmd[], uint8_t checkedBytes);

const char* dscPanelKeysArray;
volatile bool dscPanelKeyPending, dscPanelKeysPending;
bool dscWriteArm[dscPartitions];
bool dscPreviousTrouble;
bool dscPreviousKeybus;
uint8_t dscPreviousAccessCode[dscPartitions];
uint8_t dscPreviousLights[dscPartitions], dscPreviousStatus[dscPartitions];
bool dscPreviousReady[dscPartitions];
bool dscPreviousExitDelay[dscPartitions], dscPreviousEntryDelay[dscPartitions];
uint8_t dscPreviousExitState[dscPartitions];
bool dscPreviousArmed[dscPartitions], dscPreviousArmedStay[dscPartitions];
bool dscPreviousAlarm[dscPartitions];
bool dscPreviousFire[dscPartitions];
uint8_t dscPreviousOpenZones[dscZones], dscPreviousAlarmZones[dscZones];

uint8_t dscWriteByte, dscWriteBit;
uint8_t dscPanelKey;
uint8_t dscPanelBitCount, dscPanelByteCount;
volatile bool dscWriteAlarm, dscWriteAsterisk, dscWroteAsterisk;
volatile unsigned long dscClockHighTime, dscKeybusTime;
volatile uint8_t dscPanelBufferLength;
volatile uint8_t dscPanelBuffer[dscBufferSize][dscReadSize];
volatile uint8_t dscPanelBufferBitCount[dscBufferSize], dscPanelBufferByteCount[dscBufferSize];
volatile uint8_t dscCurrentCmd, dscStatusCmd;
volatile uint8_t dscIsrPanelData[dscReadSize], dscIsrPanelBitTotal, dscIsrPanelBitCount, dscIsrPanelByteCount;

#endif  // dscKeybus_h
