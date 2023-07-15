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

// dscKeybus public:

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

// Data setup - partitions, zones, and buffer size can be customized
#define dscPartitions 8    // Maximum number of partitions - requires 19 bytes of memory per partition
#define dscZones 8         // Maximum number of zone groups, 8 zones per group - requires 6 bytes of memory per zone group
#define dscBufferSize 50   // Number of commands to buffer if the sketch is busy - requires dscDataSize + 2 bytes of memory per command
#define dscDataSize 16     // Maximum bytes of a Keybus command
#define dscWriteSize 6     // Maximum types of panel commands to write data to the panel from dscWriteData[]

// Library control
void dsc_init(void);       // Initializes the library
void dsc_stop();           // Disables the clock GPIO interrupt and data timer interrupt
void dsc_reset();          // Resets the state of all status components as changed for programs to get the current status

// Virtual keypad
void dscWriteKey(int receivedKey);             // Writes a single key - nonblocking unless a previous write is in progress
void dscWriteKeys(const char *receivedKeys);  // Writes multiple keys from a char array - blocks until the write is complete
bool dscWriteReady;         // True when the library is ready to write a key
uint8_t dscWritePartition;  // Set to the partition number to use for writing keys to the panel

// Exit delay target states - these can be checked by programs using dscExitState[dscPartitions]
enum dscExitStateValues{DSC_EXIT_NONE, DSC_EXIT_STAY, DSC_EXIT_AWAY, DSC_EXIT_NO_ENTRY_DELAY};

// Panel time
bool dscTimestampChanged;          // True after the panel sends a timestamped message
uint8_t dscHour, dscMinute, dscDay, dscMonth;
int dscYear;

// Sets panel time
// Year can be sent as either 2 or 4 digits
// Set the panel user access code in dscAccessCode
// Set dscTimePartition to the partition number that should be used for writing to the panel
// Returns false if the panel is not ready
bool dscSetTime(unsigned int year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, const char* dscAccessCode, uint8_t dscTimePartition);

// Status tracking
bool dscStatusChanged;                      // True after any status change
bool dscPauseStatus;                        // Flag for use by programs to control when to update status
bool dscKeybusConnected, dscKeybusChanged;  // True if data is detected on the Keybus
uint8_t dscAccessCode[dscPartitions];
bool dscAccessCodeChanged[dscPartitions];
bool dscAccessCodePrompt;                   // True if the panel is requesting an access code
bool dscTrouble, dscTroubleChanged;
bool dscPowerTrouble, dscPowerChanged;
bool dscBatteryTrouble, dscBatteryChanged;
bool dscKeypadFireAlarm, dscKeypadAuxAlarm, dscKeypadPanicAlarm;
bool dscReady[dscPartitions], dscReadyChanged[dscPartitions];
bool dscDisabled[dscPartitions], dscDisabledChanged[dscPartitions];
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
uint8_t dscPanelData[dscDataSize];
volatile uint8_t dscWriteData[dscWriteSize][dscDataSize];  // Data to write to the panel during panel commands

// dscStatus[] and dscLights[] store the current status message and LED state for each partition.  These can be accessed
// directly in programs to get data that is not already tracked in the library.  See dscPrintPanelMessages() and
// dscPrintPanelLights() in dscKeybusProtocol.c to see how this data translates to the status message and LED status.
uint8_t dscStatus[dscPartitions];
uint8_t dscLights[dscPartitions];

// Checks if the panel data buffer is full - true if dscBufferSize needs to be increased
volatile bool dscBufferOverflow;


// dscKeybus private:

// Task handling
TaskHandle_t dscProcessHandle;
SemaphoreHandle_t dscDataAvailable;
bool dscPanelDataAvailable;

// GPIO and timer interrupt setup
#define dscClockPinMask (1ULL << dscClockPin)
#define dscReadPinMask  (1ULL << dscReadPin)
#define dscWritePinMask (1LL << dscWritePin)
#define dscTimerGroup TIMER_GROUP_0
#define dscTimerNumber TIMER_0
#define dscTimerInterval 50
#define dscTimerDivider 80
#define dscTimerISRPriority (ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED)
#define dscGPIOISRPriority (ESP_INTR_FLAG_LEVEL3 | ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_SHARED)

// Data verification - number of status commands that must match sequentially to be considered valid data and added to the panel data buffer
#define dscCommandVerifyCount 3

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
bool dscWriteAccessCode[dscPartitions];
bool dscPreviousTrouble;
bool dscPreviousKeybus;
bool dscPreviousPower;
bool dscPreviousDisabled[dscPartitions];
uint8_t dscPreviousAccessCode[dscPartitions];
uint8_t dscPreviousLights[dscPartitions], dscPreviousStatus[dscPartitions];
bool dscPreviousReady[dscPartitions];
bool dscPreviousExitDelay[dscPartitions], dscPreviousEntryDelay[dscPartitions];
uint8_t dscPreviousExitState[dscPartitions];
bool dscPreviousArmed[dscPartitions], dscPreviousArmedStay[dscPartitions], dscPreviousNoEntryDelay[dscPartitions];
bool dscPreviousAlarm[dscPartitions];
bool dscPreviousFire[dscPartitions];
uint8_t dscPreviousOpenZones[dscZones], dscPreviousAlarmZones[dscZones];
uint8_t dscPreviousPgmOutputs[2];
bool dscKeybusVersion1;

uint8_t dscWriteByte, dscWriteCommand;
char dscWriteKeyData;
uint8_t dscPanelBitCount, dscPanelByteCount;
volatile bool dscWriteKeyPending;
volatile bool dscWriteAlarm, dscStarKeyCheck, dscStarKeyWait[dscPartitions];
volatile int64_t dscClockHighTime, dscKeybusTime;
volatile uint8_t dscPanelBufferLength;
volatile uint8_t dscPanelBuffer[dscBufferSize][dscDataSize];
volatile uint8_t dscPanelBufferBitCount[dscBufferSize], dscPanelBufferByteCount[dscBufferSize];
volatile uint8_t dscIsrPanelData[dscDataSize], dscIsrPanelBitTotal, dscIsrPanelBitCount, dscIsrPanelByteCount;

#endif  // dscKeybus_h
