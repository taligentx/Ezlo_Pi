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
#define DSC_CLOCK_PIN 18
#define DSC_READ_PIN  19
#define DSC_WRITE_PIN 21

#elif defined(CONFIG_IDF_TARGET_ESP32S2)
#define DSC_CLOCK_PIN 14
#define DSC_READ_PIN  15
#define DSC_WRITE_PIN 16
#endif

// Data setup - partitions, zones, and buffer size can be customized
#define DSC_PARTITIONS 8    // Maximum number of partitions - requires 19 bytes of memory per partition
#define DSC_ZONES 8         // Maximum number of zone groups, 8 zones per group - requires 6 bytes of memory per zone group
#define DSC_BUFFER_SIZE 50  // Number of commands to buffer if the sketch is busy - requires DSC_DATA_SIZE + 2 bytes of memory per command
#define DSC_DATA_SIZE 16    // Maximum bytes of a Keybus command
#define DSC_WRITE_SIZE 6    // Maximum types of panel commands to write data to the panel from dscWriteData[]

// Library control
void dsc_init(void);        // Initializes the library
void dsc_stop();            // Disables the clock GPIO interrupt and data timer interrupt
void dsc_reset();           // Resets the state of all status components as changed for programs to get the current status

// Virtual keypad
void dscWriteKey(int receivedKey);             // Writes a single key - nonblocking unless a previous write is in progress
void dscWriteKeys(const char *receivedKeys);   // Writes multiple keys from a char array - blocks until the write is complete
bool dscWriteReady;         // True when the library is ready to write a key
uint8_t dscWritePartition;  // Set to the partition number to use for writing keys to the panel

// Exit delay target states - these can be checked by programs using dscExitState[DSC_PARTITIONS]
enum dscExitStateValues{DSC_EXIT_NONE, DSC_EXIT_STAY, DSC_EXIT_AWAY, DSC_EXIT_NO_ENTRY_DELAY};

// Panel time
bool dscTimestampChanged;    // True after the panel sends a timestamped message
uint8_t dscHour, dscMinute, dscDay, dscMonth;
int dscYear;

// Sets panel time - year can be sent as either 2 or 4 digits with an access code and the partition
// number that should be used for writing to the panel, returns false if the panel is not ready
bool dscSetTime(unsigned int year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, const char* dscAccessCode, uint8_t dscTimePartition);

// Status tracking
bool dscStatusChanged;                      // True after any status change
bool dscPauseStatus;                        // Flag for use by programs to control when to update status
bool dscKeybusConnected, dscKeybusChanged;  // True if data is detected on the Keybus
uint8_t dscAccessCode[DSC_PARTITIONS];
bool dscAccessCodeChanged[DSC_PARTITIONS];
bool dscAccessCodePrompt;                   // True if the panel is requesting an access code
bool dscTrouble, dscTroubleChanged;
bool dscPowerTrouble, dscPowerChanged;
bool dscBatteryTrouble, dscBatteryChanged;
bool dscKeypadFireAlarm, dscKeypadAuxAlarm, dscKeypadPanicAlarm;
bool dscReady[DSC_PARTITIONS], dscReadyChanged[DSC_PARTITIONS];
bool dscDisabled[DSC_PARTITIONS], dscDisabledChanged[DSC_PARTITIONS];
bool dscArmed[DSC_PARTITIONS], dscArmedAway[DSC_PARTITIONS], dscArmedStay[DSC_PARTITIONS];
bool dscNoEntryDelay[DSC_PARTITIONS], dscArmedChanged[DSC_PARTITIONS];
bool dscAlarm[DSC_PARTITIONS], dscAlarmChanged[DSC_PARTITIONS];
bool dscExitDelay[DSC_PARTITIONS], dscExitDelayChanged[DSC_PARTITIONS];
uint8_t dscExitState[DSC_PARTITIONS], dscExitStateChanged[DSC_PARTITIONS];
bool dscEntryDelay[DSC_PARTITIONS], dscEntryDelayChanged[DSC_PARTITIONS];
bool dscFire[DSC_PARTITIONS], dscFireChanged[DSC_PARTITIONS];
bool dscOpenZonesStatusChanged;
uint8_t dscOpenZones[DSC_ZONES], dscOpenZonesChanged[DSC_ZONES];    // Zone status is stored in an array using 1 bit per zone, up to 64 zones
bool dscAlarmZonesStatusChanged;
uint8_t dscAlarmZones[DSC_ZONES], dscAlarmZonesChanged[DSC_ZONES];  // Zone alarm status is stored in an array using 1 bit per zone, up to 64 zones
bool dscPgmOutputsStatusChanged;
uint8_t dscPgmOutputs[2], dscPgmOutputsChanged[2];
uint8_t dscPanelVersion;

// dscPanelData[] and dscWriteData[] store panel data and module data to write in an array: command [0],
// stop bit by itself [1], followed by the remaining data.  These can be accessed directly in the program
// to get data that is not already tracked in the library.  See dscKeybusProtocol.c for the currently
// known DSC commands and data.
//
// dscPanelData[] example:
//   Byte 0     Byte 2   Byte 3   Byte 4   Byte 5
//   00000101 0 10000001 00000001 10010001 11000111 [0x05] Partition 1: Ready Backlight - Partition ready | Partition 2: disabled
//            ^ Byte 1 (stop bit)
uint8_t dscPanelData[DSC_DATA_SIZE];
volatile uint8_t dscWriteData[DSC_WRITE_SIZE][DSC_DATA_SIZE];  // Data to write to the panel during panel commands

// dscStatus[] and dscLights[] store the current status message and LED state for each partition.  These can be accessed
// directly in programs to get data that is not already tracked in the library.  See dscPrintPanelMessages() and
// dscPrintPanelLights() in dscKeybusProtocol.c to see how this data translates to the status message and LED status.
uint8_t dscStatus[DSC_PARTITIONS];
uint8_t dscLights[DSC_PARTITIONS];

// Checks if the panel data buffer is full - true if DSC_BUFFER_SIZE needs to be increased
volatile bool dscBufferOverflow;


// dscKeybus private

// Task handling
TaskHandle_t dscProcessHandle;
SemaphoreHandle_t dscDataAvailable;
bool dscPanelDataAvailable;

// GPIO and timer interrupt setup
#define DSC_CLOCK_PIN_MASK (1ULL << DSC_CLOCK_PIN)
#define DSC_READ_PIN_MASK  (1ULL << DSC_READ_PIN)
#define DSC_WRITE_PIN_MASK (1LL << DSC_WRITE_PIN)
#define DSC_TIMER_GROUP TIMER_GROUP_0
#define DSC_TIMER_NUMBER TIMER_0
#define DSC_TIMER_INTERVAL 50
#define DSC_TIMER_DIVIDER 80
#define DSC_TIMER_ISR_PRIORITY (ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED)
#define DSC_GPIO_ISR_PRIORITY (ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED)

// Data verification - number of status commands that must match sequentially to be considered valid data and added to the panel data buffer
#define DSC_COMMAND_VERIFY_COUNT 3

// Syntax compatibility wrappers
#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

// Status processing
void dscProcessPanelStatus();
void dscProcessPanelStatus0(uint8_t partition, uint8_t panelByte);
void dscProcessPanelStatus1(uint8_t partition, uint8_t panelByte);
void dscProcessPanelStatus2(uint8_t partition, uint8_t panelByte);
void dscProcessPanelStatus4(uint8_t partition, uint8_t panelByte);
void dscProcessPanelStatus5(uint8_t partition, uint8_t panelByte);
void dscProcessPanel_0x16();
void dscProcessPanel_0x27();
void dscProcessPanel_0x2D();
void dscProcessPanel_0x34();
void dscProcessPanel_0x3E();
void dscProcessPanel_0x87();
void dscProcessPanel_0xA5();
void dscProcessPanel_0xE6();
void dscProcessPanel_0xE6_0x09();
void dscProcessPanel_0xE6_0x0B();
void dscProcessPanel_0xE6_0x0D();
void dscProcessPanel_0xE6_0x0F();
void dscProcessPanel_0xE6_0x1A();
void dscProcessPanel_0xEB();
void dscProcessReadyStatus(uint8_t partitionIndex, bool status);
void dscProcessAlarmStatus(uint8_t partitionIndex, bool status);
void dscProcessExitDelayStatus(uint8_t partitionIndex, bool status);
void dscProcessEntryDelayStatus(uint8_t partitionIndex, bool status);
void dscProcessNoEntryDelayStatus(uint8_t partitionIndex, bool status);
void dscProcessZoneStatus(uint8_t zonesByte, uint8_t panelByte);
void dscProcessTime(uint8_t panelByte);
void dscProcessAlarmZones(uint8_t panelByte, uint8_t startByte, uint8_t zoneCountOffset, uint8_t writeValue);
void dscProcessAlarmZonesStatus(uint8_t zonesByte, uint8_t zoneCount, uint8_t writeValue);
void dscProcessArmed(uint8_t partitionIndex, bool armedStatus);
void dscProcessPanelAccessCode(uint8_t partitionIndex, uint8_t dscCode, bool accessCodeIncrease);

bool dscValidCRC();
void dscSetWriteKey(int receivedKey);
bool dscRedundantPanelData(uint8_t previousCmd[], volatile uint8_t currentCmd[], uint8_t checkedBytes);

bool dscWriteKeysPending;
bool dscWriteAccessCode[DSC_PARTITIONS];
bool dscPreviousTrouble;
bool dscPreviousKeybus;
bool dscPreviousPower;
bool dscPreviousDisabled[DSC_PARTITIONS];
uint8_t dscPreviousAccessCode[DSC_PARTITIONS];
uint8_t dscPreviousLights[DSC_PARTITIONS], dscPreviousStatus[DSC_PARTITIONS];
bool dscPreviousReady[DSC_PARTITIONS];
bool dscPreviousExitDelay[DSC_PARTITIONS], dscPreviousEntryDelay[DSC_PARTITIONS];
uint8_t dscPreviousExitState[DSC_PARTITIONS];
bool dscPreviousArmed[DSC_PARTITIONS], dscPreviousArmedStay[DSC_PARTITIONS], dscPreviousNoEntryDelay[DSC_PARTITIONS];
bool dscPreviousAlarm[DSC_PARTITIONS];
bool dscPreviousFire[DSC_PARTITIONS];
uint8_t dscPreviousOpenZones[DSC_ZONES], dscPreviousAlarmZones[DSC_ZONES];
uint8_t dscPreviousPgmOutputs[2];
bool dscKeybusVersion1;

uint8_t dscWriteByte, dscWriteCommand;
char dscWriteKeyData;
uint8_t dscPanelBitCount, dscPanelByteCount;
volatile bool dscWriteKeyPending;
volatile bool dscWriteAlarm, dscStarKeyCheck, dscStarKeyWait[DSC_PARTITIONS];
volatile int64_t dscClockHighTime, dscKeybusTime;
volatile uint8_t dscPanelBufferLength;
volatile uint8_t dscPanelBuffer[DSC_BUFFER_SIZE][DSC_DATA_SIZE];
volatile uint8_t dscPanelBufferBitCount[DSC_BUFFER_SIZE], dscPanelBufferByteCount[DSC_BUFFER_SIZE];
volatile uint8_t dscIsrPanelData[DSC_DATA_SIZE], dscIsrPanelBitTotal, dscIsrPanelBitCount, dscIsrPanelByteCount;

#endif  // dscKeybus_h
