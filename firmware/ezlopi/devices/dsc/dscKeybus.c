#include "dscKeybus.h"


// Handler for critical sections
static portMUX_TYPE dscSpinlock = portMUX_INITIALIZER_UNLOCKED;


// Checks for changes to the panel status
void dscLoop() {
  while(1) {

    // Blocks this task until valid panel data is available
    xSemaphoreTake(dscDataAvailable, portMAX_DELAY);

    // If the Keybus data buffer is exceeded, the program is too busy to process all Keybus commands.  Call
    // dscLoop() more often, or increase dscBufferSize in the library: dscKeybus.h
    if (dscBufferOverflow) {
      TRACE_E("DSC Keybus buffer overflow");
      dscBufferOverflow = false;
    }

    if (dscStatusChanged) {      // Checks if the security system status has changed
      dscStatusChanged = false;  // Reset the status tracking flag

      // Checks if the interface is connected to the Keybus
      if (dscKeybusChanged) {
        dscKeybusChanged = false;                 // Resets the Keybus data status flag
        if (dscKeybusConnected) TRACE_I("Keybus connected");
        else TRACE_I("Keybus disconnected");
      }

      // Notifies if an access code is needed by the panel for arming or command outputs
      if (dscAccessCodePrompt) {
        dscAccessCodePrompt = false;
        TRACE_I("Enter access code");
      }

      // Checks status per partition
      for (uint8_t partition = 0; partition < dscPartitions; partition++) {

        // Checks ready status
        if (dscReadyChanged[partition]) {
          dscReadyChanged[partition] = false;  // Resets the partition ready status flag
          if (dscReady[partition]) {
            TRACE_I("Partition %d ready", partition + 1);
          }
          else {
            TRACE_I("Partition %d not ready", partition + 1);
          }
        }

        // Checks armed status
        if (dscArmedChanged[partition]) {
          dscArmedChanged[partition] = false;  // Resets the partition armed status flag
          if (dscArmed[partition]) {
            if (dscNoEntryDelay[partition]) {
              if (dscArmedAway[partition]) TRACE_I("Partition %d armed away with no entry delay", partition + 1);
              if (dscArmedStay[partition]) TRACE_I("Partition %d armed stay with no entry delay", partition + 1);
            }
            else {
              if (dscArmedAway[partition]) TRACE_I("Partition %d armed away", partition + 1);
              if (dscArmedStay[partition]) TRACE_I("Partition %d armed stay", partition + 1);
            }
          }
          else {
            TRACE_I("Partition %d disarmed", partition + 1);
          }
        }

        // Checks alarm triggered status
        if (dscAlarmChanged[partition]) {
          dscAlarmChanged[partition] = false;  // Resets the partition alarm status flag
          if (dscAlarm[partition]) {
            TRACE_I("Partition %d in alarm", partition + 1);
          }
        }

        // Checks exit delay status
        if (dscExitDelayChanged[partition]) {
          dscExitDelayChanged[partition] = false;  // Resets the exit delay status flag
          if (dscExitDelay[partition]) {
            TRACE_I("Partition %d exit delay in progress", partition + 1);
          }
          else if (!dscArmed[partition]) {  // Checks for disarm during exit delay
            TRACE_I("Partition %d disarmed", partition + 1);
          }
        }

        // Checks entry delay status
        if (dscEntryDelayChanged[partition]) {
          dscEntryDelayChanged[partition] = false;  // Resets the exit delay status flag
          if (dscEntryDelay[partition]) {
            TRACE_I("Partition %d entry delay in progress", partition + 1);
          }
        }

        // Checks the access code used to arm or disarm
        if (dscAccessCodeChanged[partition]) {
          dscAccessCodeChanged[partition] = false;  // Resets the access code status flag
          switch (dscAccessCode[partition]) {
            case 33:
            case 34: TRACE_I("Partition %d duress code %d", partition + 1, dscAccessCode[partition]); break;
            case 40: TRACE_I("Partition %d master code %d", partition + 1, dscAccessCode[partition]); break;
            case 41:
            case 42: TRACE_I("Partition %d supervisor code %d", partition + 1, dscAccessCode[partition]); break;
            default: TRACE_I("Partition %d user code %d", partition + 1, dscAccessCode[partition]);break;
          }
        }

        // Checks fire alarm status
        if (dscFireChanged[partition]) {
          dscFireChanged[partition] = false;  // Resets the fire status flag
          if (dscFire[partition]) {
            TRACE_I("Partition %d fire alarm on", partition + 1);
          }
          else {
            TRACE_I("Partition %d fire alarm restored", partition + 1);
          }
        }
      }

      // Checks for open zones
      // Zone status is stored in the openZones[] and openZonesChanged[] arrays using 1 bit per zone, up to 64 zones
      //   openZones[0] and openZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
      //   openZones[1] and openZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
      //   ...
      //   openZones[7] and openZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
      if (dscOpenZonesStatusChanged) {
        dscOpenZonesStatusChanged = false;                           // Resets the open zones status flag
        for (uint8_t zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
          for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
            if (bitRead(dscOpenZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual open zone status flag
              bitWrite(dscOpenZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual open zone status flag
              if (bitRead(dscOpenZones[zoneGroup], zoneBit)) {       // Zone open
                TRACE_I("Zone open: %d", zoneBit + 1 + (zoneGroup * 8));  // Determines the zone number
              }
              else {                                                  // Zone closed
                TRACE_I("Zone restored: %d", zoneBit + 1 + (zoneGroup * 8));  // Determines the zone number
              }
            }
          }
        }
      }

      // Checks for zones in alarm
      // Zone alarm status is stored in the alarmZones[] and alarmZonesChanged[] arrays using 1 bit per zone, up to 64 zones
      //   alarmZones[0] and alarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
      //   alarmZones[1] and alarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
      //   ...
      //   alarmZones[7] and alarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
      if (dscAlarmZonesStatusChanged) {
        dscAlarmZonesStatusChanged = false;                           // Resets the alarm zones status flag
        for (uint8_t zoneGroup = 0; zoneGroup < dscZones; zoneGroup++) {
          for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
            if (bitRead(dscAlarmZonesChanged[zoneGroup], zoneBit)) {  // Checks an individual alarm zone status flag
              bitWrite(dscAlarmZonesChanged[zoneGroup], zoneBit, 0);  // Resets the individual alarm zone status flag
              if (bitRead(dscAlarmZones[zoneGroup], zoneBit)) {       // Zone alarm
                TRACE_I("Zone alarm: %d", zoneBit + 1 + (zoneGroup * 8));  // Determines the zone number
              }
              else {
                TRACE_I("Zone alarm restored: %d", zoneBit + 1 + (zoneGroup * 8));  // Determines the zone number
              }
            }
          }
        }
      }

      // PGM outputs 1-14 status is stored in the dscPgmOutputs[] and dscPgmOutputsChanged[] arrays using 1 bit per PGM output:
      //   dscPgmOutputs[0] and dscPgmOutputsChanged[0]: Bit 0 = PGM 1 ... Bit 7 = PGM 8
      //   dscPgmOutputs[1] and dscPgmOutputsChanged[1]: Bit 0 = PGM 9 ... Bit 5 = PGM 14
      if (dscPgmOutputsStatusChanged) {
        dscPgmOutputsStatusChanged = false;  // Resets the PGM outputs status flag
        for (uint8_t pgmGroup = 0; pgmGroup < 2; pgmGroup++) {
          for (uint8_t pgmBit = 0; pgmBit < 8; pgmBit++) {
            if (bitRead(dscPgmOutputsChanged[pgmGroup], pgmBit)) {         // Checks an individual PGM output status flag
              bitWrite(dscPgmOutputsChanged[pgmGroup], pgmBit, 0);         // Resets the individual PGM output status flag
              if (bitRead(dscPgmOutputs[pgmGroup], pgmBit)) {             // PGM enabled
                TRACE_I("PGM enabled: %d", pgmBit + 1 + (pgmGroup * 8));   // Determines the PGM output number
              }
              else {
                TRACE_I("PGM disabled: %d", pgmBit + 1 + (pgmGroup * 8));  // Determines the PGM output number
              }
            }
          }
        }
      }

      // Checks for a panel timestamp
      //
      // The panel time can be set using dsc.setTime(year, month, day, hour, minute, "accessCode") - for example:
      //   dsc.setTime(2015, 10, 21, 7, 28, "1234")  # Sets 2015.10.21 07:28 with access code 1234
      //
      if (dscTimestampChanged) {
        dscTimestampChanged = false;
        TRACE_I("Timestamp: %d.%02d.%02d %02d:%02d", dscYear, dscMonth, dscDay, dscHour, dscMinute);
      }

      // Checks trouble status
      if (dscTroubleChanged) {
        dscTroubleChanged = false;  // Resets the trouble status flag
        if (dscTrouble) TRACE_I("Trouble status on");
        else TRACE_I("Trouble status restored");
      }

      // Checks AC power status
      if (dscPowerChanged) {
        dscPowerChanged = false;  // Resets the power trouble status flag
        if (dscPowerTrouble) TRACE_I("Panel AC power trouble");
        else TRACE_I("Panel AC power restored");
      }

      // Checks panel battery status
      if (dscBatteryChanged) {
        dscBatteryChanged = false;  // Resets the battery trouble status flag
        if (dscBatteryTrouble) TRACE_I("Panel battery trouble");
        else TRACE_I("Panel battery restored");
      }

      // Checks keypad fire alarm triggered
      if (dscKeypadFireAlarm) {
        dscKeypadFireAlarm = false;  // Resets the keypad fire alarm status flag
        TRACE_I("Keypad fire alarm");
      }

      // Checks keypad auxiliary alarm triggered
      if (dscKeypadAuxAlarm) {
        dscKeypadAuxAlarm = false;  // Resets the keypad auxiliary alarm status flag
        TRACE_I("Keypad aux alarm");
      }

      // Checks keypad panic alarm triggered
      if (dscKeypadPanicAlarm) {
        dscKeypadPanicAlarm = false;  // Resets the keypad panic alarm status flag
        TRACE_I("Keypad panic alarm");
      }
    }
  }
}


// Checks panel data CRC
bool IRAM_ATTR dscValidCRC() {
  uint8_t byteCount = (dscPanelBitCount - 1) / 8;
  int dataSum = 0;
  for (uint8_t dscPanelByte = 0; dscPanelByte < byteCount; dscPanelByte++) {
    if (dscPanelByte != 1) dataSum += dscPanelData[dscPanelByte];
  }
  if (dataSum % 256 == dscPanelData[byteCount]) return true;
  else return false;
}


// Checks for redundant panel data from dscPanelLoop()
bool dscRedundantPanelData(uint8_t dscPreviousCmd[], volatile uint8_t dscCurrentCmd[], uint8_t checkedBytes) {
  bool redundantData = true;
  for (uint8_t i = 0; i < checkedBytes; i++) {
    if (dscPreviousCmd[i] != dscCurrentCmd[i]) {
      redundantData = false;
      break;
    }
  }
  if (redundantData) return true;
  else {
    for (uint8_t i = 0; i < dscReadSize; i++) dscPreviousCmd[i] = dscCurrentCmd[i];
    return false;
  }
}


// Checks for redundant panel status data from dscDataInterrupt()
static bool IRAM_ATTR dscRedundantPanelDataISR(uint8_t dscPreviousCmd[], volatile uint8_t dscCurrentCmd[], uint8_t checkedBytes) {
  static uint8_t countCmd05 = 0, countCmd1B = 0;
  static uint8_t pendingCmd05[dscReadSize], pendingCmd1B[dscReadSize];
  bool pendingCmd05Mismatch = false, pendingCmd1BMismatch = false;
  bool redundantData = true;

  // 0x05 and 0x1B status commands do not contain a CRC - this data is instead verified by
  // requiring dscCommandVerifyCount number of identical messages before accepting the new
  // status data as valid
  for (uint8_t i = 0; i < checkedBytes; i++) {
    if (dscPreviousCmd[i] != dscCurrentCmd[i]) {
      if (dscCurrentCmd[0] == 0x05) {

        if (countCmd05 >= dscCommandVerifyCount) {
          redundantData = false;
          countCmd05 = 0;
        }
        else {
          if (countCmd05 == 0) {
            for (uint8_t j = 0; j < dscReadSize; j++) pendingCmd05[j] = dscCurrentCmd[j];
            countCmd05++;
          }
          else {
            for (uint8_t j = 0; j < checkedBytes; j++) {
              if (pendingCmd05[j] != dscCurrentCmd[j]) pendingCmd05Mismatch = true;
            }
            if (pendingCmd05Mismatch) {
              for (uint8_t j = 0; j < dscReadSize; j++) pendingCmd05[j] = dscCurrentCmd[j];
              countCmd05 = 0;
            }
            else countCmd05++;
          }
        }

      }
      else if (dscCurrentCmd[0] == 0x1B) {
        if (countCmd1B >= dscCommandVerifyCount) {
          redundantData = false;
          countCmd1B = 0;
        }
        else {
          if (countCmd1B == 0) {
            for (uint8_t j = 0; j < dscReadSize; j++) pendingCmd1B[j] = dscCurrentCmd[j];
            countCmd1B++;
          }
          else {
            for (uint8_t j = 0; j < checkedBytes; j++) {
              if (pendingCmd1B[j] != dscCurrentCmd[j]) pendingCmd1BMismatch = true;
            }
            if (pendingCmd1BMismatch) {
              for (uint8_t j = 0; j < dscReadSize; j++) pendingCmd1B[j] = dscCurrentCmd[j];
              countCmd1B = 0;
            }
            else countCmd1B++;
          }
        }
      }

      break;
    }
  }

  if (redundantData) return true;
  else {
    for (uint8_t i = 0; i < dscReadSize; i++) dscPreviousCmd[i] = dscCurrentCmd[i];
    return false;
  }
}


// Gets GPIO pin level for ISRs running from IRAM
static int IRAM_ATTR dscGetLevelISR(gpio_num_t gpioPin) {
    return (GPIO.in >> gpioPin) & 0x1;
}


// Sets GPIO pin level for ISRs running from IRAM
static void IRAM_ATTR dscSetLevelISR(gpio_num_t gpioPin, uint8_t gpioLevel)
{
    if (gpioLevel) GPIO.out_w1ts = (1 << gpioPin);
    else GPIO.out_w1tc = (1 << gpioPin);
}


// Called as an interrupt when the DSC clock changes to write data for virtual keypad and setup timers to read
// data after an interval.
static void IRAM_ATTR dscClockInterrupt() {

  // Sets up a timer that will call dscDataInterrupt() after dscTimerInterval to read the data line.
  // Data sent from the panel and keypads/modules has latency after a clock change (observed up to 160us for keypad data).
  timer_group_set_counter_enable_in_isr(dscTimerGroup, dscTimerNumber, TIMER_START);
  portENTER_CRITICAL(&dscSpinlock);

  static int64_t dscPreviousClockHighTime;

  if (dscGetLevelISR(dscClockPin) == 1) {
    dscSetLevelISR(dscWritePin, 0);  // Restores the data line after a virtual keypad write
    dscPreviousClockHighTime = esp_timer_get_time();
  }

  else {
    dscClockHighTime = esp_timer_get_time() - dscPreviousClockHighTime;  // Tracks the clock high time to find the reset between commands

    // Virtual keypad
    static bool writeStart = false;
    static bool writeRepeat = false;
    static bool writeCmd = false;

    if (dscWritePartition <= 4 && dscStatusCmd == 0x05) writeCmd = true;
    else if (dscWritePartition > 4 && dscStatusCmd == 0x1B) writeCmd = true;
    else writeCmd = false;

    // Writes a F/A/P alarm key and repeats the key on the next immediate command from the panel (0x1C verification)
    if ((dscWriteAlarm && dscPanelKeyPending) || writeRepeat) {

      // Writes the first bit by shifting the alarm key data right 7 bits and checking bit 0
      if (dscIsrPanelBitTotal == 1) {
        if (!((dscPanelKey >> 7) & 0x01)) {
          dscSetLevelISR(dscWritePin, 1);
        }
        writeStart = true;  // Resolves a timing issue where some writes do not begin at the correct bit
      }

      // Writes the remaining alarm key data
      else if (writeStart && dscIsrPanelBitTotal > 1 && dscIsrPanelBitTotal <= 8) {
        if (!((dscPanelKey >> (8 - dscIsrPanelBitTotal)) & 0x01)) dscSetLevelISR(dscWritePin, 1);

        // Resets counters when the write is complete
        if (dscIsrPanelBitTotal == 8) {
          dscPanelKeyPending = false;
          writeStart = false;
          dscWriteAlarm = false;

          // Sets up a repeated write for alarm keys
          if (!writeRepeat) writeRepeat = true;
          else writeRepeat = false;
        }
      }
    }

    // Writes a regular key unless waiting for a response to the '*' key or the panel is sending a query command
    else if (dscPanelKeyPending && !dscWroteAsterisk && dscIsrPanelByteCount == dscWriteByte && writeCmd) {

      // Writes the first bit by shifting the key data right 7 bits and checking bit 0
      if (dscIsrPanelBitTotal == dscWriteBit) {
        if (!((dscPanelKey >> 7) & 0x01)) dscSetLevelISR(dscWritePin, 1);
        writeStart = true;  // Resolves a timing issue where some writes do not begin at the correct bit
      }

      // Writes the remaining alarm key data
      else if (writeStart && dscIsrPanelBitTotal > dscWriteBit && dscIsrPanelBitTotal <= dscWriteBit + 7) {
        if (!((dscPanelKey >> (7 - dscIsrPanelBitCount)) & 0x01)) dscSetLevelISR(dscWritePin, 1);

        // Resets counters when the write is complete
        if (dscIsrPanelBitTotal == dscWriteBit + 7) {
          if (dscWriteAsterisk) dscWroteAsterisk = true;  // Delays writing after pressing '*' until the panel is ready
          else dscPanelKeyPending = false;
          writeStart = false;
        }
      }
    }
  }
  portEXIT_CRITICAL(&dscSpinlock);
}


// Timer interrupt called by dscClockInterrupt() after dscTimerInterval to read the data line
// Data sent from the panel and keypads/modules has latency after a clock change (observed up to 160us for keypad data).
static bool IRAM_ATTR dscDataInterrupt(void *args) {
  timer_group_set_counter_enable_in_isr(dscTimerGroup, dscTimerNumber, TIMER_PAUSE);
  portENTER_CRITICAL(&dscSpinlock);

  static bool skipData = false;

  // Panel sends data while the clock is high
  if (dscGetLevelISR(dscClockPin) == 1) {

    // Stops processing Keybus data at the dscReadSize limit
    if (dscIsrPanelByteCount >= dscReadSize) skipData = true;

    else {
      if (dscIsrPanelBitCount < 8) {

        // Data is captured in each byte by shifting left by 1 bit and writing to bit 0
        dscIsrPanelData[dscIsrPanelByteCount] <<= 1;
        if (dscGetLevelISR(dscReadPin) == 1) {
          dscIsrPanelData[dscIsrPanelByteCount] |= 1;
        }
      }

      if (dscIsrPanelBitTotal == 8) {

        // Tests for a status command, used in dscClockInterrupt() to ensure keys are only written during a status command
        switch (dscIsrPanelData[0]) {
          case 0x05:
          case 0x0A: dscStatusCmd = 0x05; break;
          case 0x1B: dscStatusCmd = 0x1B; break;
          default: dscStatusCmd = 0; break;
        }

        // Stores the stop bit by itself in byte 1 - this aligns the Keybus bytes with dscPanelData[] bytes
        dscIsrPanelBitCount = 0;
        dscIsrPanelByteCount++;
      }

      // Increments the bit counter if the byte is incomplete
      else if (dscIsrPanelBitCount < 7) {
        dscIsrPanelBitCount++;
      }

      // Byte is complete, set the counters for the next byte
      else {
        dscIsrPanelBitCount = 0;
        dscIsrPanelByteCount++;
      }

      dscIsrPanelBitTotal++;
    }
  }

  // Keypads and modules send data while the clock is low
  else {

    // Saves data and resets counters after the clock cycle is complete (high for at least 1ms)
    if (dscClockHighTime > 1000) {
      dscKeybusTime = esp_timer_get_time();

      // Skips incomplete and redundant data from status commands - these are sent constantly on the keybus at a high
      // rate, so they are always skipped.  Checking is required in the ISR to prevent flooding the buffer.
      if (dscIsrPanelBitTotal < 8) skipData = true;
      else switch (dscIsrPanelData[0]) {
        static uint8_t dscPreviousCmd05[dscReadSize];
        static uint8_t dscPreviousCmd1B[dscReadSize];
        case 0x05:  // Status: partitions 1-4
          if (dscRedundantPanelDataISR(dscPreviousCmd05, dscIsrPanelData, dscIsrPanelByteCount)) skipData = true;
          break;

        case 0x1B:  // Status: partitions 5-8
          if (dscRedundantPanelDataISR(dscPreviousCmd1B, dscIsrPanelData, dscIsrPanelByteCount)) skipData = true;
          break;
      }

      // Stores new panel data in the panel buffer
      dscCurrentCmd = dscIsrPanelData[0];
      if (dscPanelBufferLength == dscBufferSize) dscBufferOverflow = true;
      else if (!skipData && dscPanelBufferLength < dscBufferSize) {
        for (uint8_t i = 0; i < dscReadSize; i++) dscPanelBuffer[dscPanelBufferLength][i] = dscIsrPanelData[i];
        dscPanelBufferBitCount[dscPanelBufferLength] = dscIsrPanelBitTotal;
        dscPanelBufferByteCount[dscPanelBufferLength] = dscIsrPanelByteCount;
        dscPanelBufferLength++;
      }

      // Resets the panel capture data and counters
      for (uint8_t i = 0; i < dscReadSize; i++) dscIsrPanelData[i] = 0;
      dscIsrPanelBitTotal = 0;
      dscIsrPanelBitCount = 0;
      dscIsrPanelByteCount = 0;
      skipData = false;

      // Notifies the dscPanelLoop task when new data is available
      if (dscPanelBufferLength > 0) {
        BaseType_t xHigherPriorityTaskWoken;
        vTaskNotifyGiveFromISR(dscPanelLoopHandle, &xHigherPriorityTaskWoken);
        if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
      }
    }
  }

  portEXIT_CRITICAL(&dscSpinlock);
  return true;
}


void dscPanelLoop() {

  // Sets this task to be notified by dscDataInterrupt() when new data is available
  dscPanelLoopHandle = xTaskGetCurrentTaskHandle();

  while(1) {

    // Waits until notification from dscDataInterrupt() that new data is available
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

    // Checks if Keybus data is detected and sets a status flag if data is not detected for 3s
    if (esp_timer_get_time() - dscKeybusTime > 3000000) dscKeybusConnected = false;  // dscKeybusTime is set in dscDataInterrupt() when the clock resets
    else dscKeybusConnected = true;

    if (dscPreviousKeybus != dscKeybusConnected) {
      dscPreviousKeybus = dscKeybusConnected;
      dscKeybusChanged = true;
      if (!dscPauseStatus) dscStatusChanged = true;
      if (!dscKeybusConnected) continue;
    }

    // Copies data from the buffer to dscPanelData[]
    static uint8_t dscPanelBufferIndex = 1;
    uint8_t dataIndex = dscPanelBufferIndex - 1;
    for (uint8_t i = 0; i < dscReadSize; i++) dscPanelData[i] = dscPanelBuffer[dataIndex][i];
    dscPanelBitCount = dscPanelBufferBitCount[dataIndex];
    dscPanelByteCount = dscPanelBufferByteCount[dataIndex];
    dscPanelBufferIndex++;

    // Resets counters when the buffer is cleared
    taskENTER_CRITICAL(&dscSpinlock);
    if (dscPanelBufferIndex > dscPanelBufferLength) {
      dscPanelBufferIndex = 1;
      dscPanelBufferLength = 0;
    }
    taskEXIT_CRITICAL(&dscSpinlock);

    // Waits at startup for the 0x05 status command or a command with valid CRC data to eliminate spurious data.
    static bool firstClockCycle = true;
    if (firstClockCycle) {
      if ((dscValidCRC() || dscPanelData[0] == 0x05) && dscPanelData[0] != 0) firstClockCycle = false;
      else continue;
    }

    // Skips redundant data sent constantly while in installer programming
    static uint8_t dscPreviousCmd0A[dscReadSize], dscPreviousCmd0F[dscReadSize], dscPreviousCmdE6_20[dscReadSize], dscPreviousCmdE6_21[dscReadSize];
    switch (dscPanelData[0]) {
      case 0x0A:  // Partition 1 status in programming
        if (dscRedundantPanelData(dscPreviousCmd0A, dscPanelData, dscReadSize)) continue;
        break;

      case 0x0F:  // Partition 2 status in programming
        if (dscRedundantPanelData(dscPreviousCmd0F, dscPanelData, dscReadSize)) continue;
        break;

      case 0xE6:
        if (dscPanelData[2] == 0x20 && dscRedundantPanelData(dscPreviousCmdE6_20, dscPanelData, dscReadSize)) continue;  // Partition 1 status in programming, zone lights 33-64
        if (dscPanelData[2] == 0x21 && dscRedundantPanelData(dscPreviousCmdE6_21, dscPanelData, dscReadSize)) continue;  // Partition 2 status in programming, zone lights 33-64
        break;
    }
    if (dscPartitions > 4) {
      static uint8_t dscPreviousCmdE6_03[dscReadSize];
      if (dscPanelData[0] == 0xE6 && dscPanelData[2] == 0x03 && dscRedundantPanelData(dscPreviousCmdE6_03, dscPanelData, 8)) continue;  // Status in alarm/programming, partitions 5-8
    }

    // Processes valid panel data
    switch (dscPanelData[0]) {
      case 0x05:
      case 0x1B: dscProcessPanelStatus(); break;
      case 0x27: dscProcessPanel_0x27(); break;
      case 0x2D: dscProcessPanel_0x2D(); break;
      case 0x34: dscProcessPanel_0x34(); break;
      case 0x3E: dscProcessPanel_0x3E(); break;
      case 0x87: dscProcessPanel_0x87(); break;
      case 0xA5: dscProcessPanel_0xA5(); break;
      case 0xE6: if (dscPartitions > 2) dscProcessPanel_0xE6(); break;
      case 0xEB: if (dscPartitions > 2) dscProcessPanel_0xEB(); break;
    }

    dscPanelDataAvailable = true;
    xSemaphoreGive(dscDataAvailable);
  }
}


// Sets up writes for a single key
void dscWriteKey(int receivedKey) {
  while(dscPanelKeyPending || dscPanelKeysPending) vTaskDelay(1);
  dscSetWriteKey(receivedKey);
}


// Sets up writes for multiple keys sent as a char array
void dscWriteKeys(const char *receivedKeys) {
  while(dscPanelKeyPending || dscPanelKeysPending) vTaskDelay(1);
  dscPanelKeysArray = receivedKeys;
  if (dscPanelKeysArray[0] != '\0') dscPanelKeysPending = true;
  while (dscPanelKeysPending) {
    static uint8_t writeCounter = 0;
    if (!dscPanelKeyPending && writeCounter < strlen(dscPanelKeysArray)) {
      if (dscPanelKeysArray[writeCounter] != '\0') {
        dscSetWriteKey(dscPanelKeysArray[writeCounter]);
        writeCounter++;
        if (dscPanelKeysArray[writeCounter] == '\0') {
          dscPanelKeysPending = false;
          writeCounter = 0;
        }
      }
    }
    else vTaskDelay(1);
  }
}


// Specifies the key value to be written by dscClockInterrupt() and selects the write partition.  This includes a 500ms
// delay after alarm keys to resolve errors when additional keys are sent immediately after alarm keys.
void dscSetWriteKey(int receivedKey) {
  static int64_t previousTime;
  static bool setPartition;

  // Sets the write partition if set by virtual keypad key '/'
  if (setPartition) {
    setPartition = false;
    if (receivedKey >= '1' && receivedKey <= '8') {
      dscWritePartition = receivedKey - 48;
    }
    return;
  }

  // Sets the binary to write for virtual keypad keys
  if (!dscPanelKeyPending && esp_timer_get_time() - previousTime > 500000) {
    bool validKey = true;
    switch (receivedKey) {
      case '/': setPartition = true; validKey = false; break;
      case '0': dscPanelKey = 0x00; break;
      case '1': dscPanelKey = 0x05; break;
      case '2': dscPanelKey = 0x0A; break;
      case '3': dscPanelKey = 0x0F; break;
      case '4': dscPanelKey = 0x11; break;
      case '5': dscPanelKey = 0x16; break;
      case '6': dscPanelKey = 0x1B; break;
      case '7': dscPanelKey = 0x1C; break;
      case '8': dscPanelKey = 0x22; break;
      case '9': dscPanelKey = 0x27; break;
      case '*': dscPanelKey = 0x28; dscWriteAsterisk = true; break;
      case '#': dscPanelKey = 0x2D; break;
      case 'f': case 'F': dscPanelKey = 0x77; dscWriteAlarm = true; break;                       // Keypad fire alarm
      case 's': case 'S': dscPanelKey = 0xAF; dscWriteArm[dscWritePartition - 1] = true; break;  // Arm stay
      case 'w': case 'W': dscPanelKey = 0xB1; dscWriteArm[dscWritePartition - 1] = true; break;  // Arm away
      case 'n': case 'N': dscPanelKey = 0xB6; dscWriteArm[dscWritePartition - 1] = true; break;  // Arm with no entry delay (night arm)
      case 'a': case 'A': dscPanelKey = 0xBB; dscWriteAlarm = true; break;                       // Keypad auxiliary alarm
      case 'c': case 'C': dscPanelKey = 0xBB; break;                                             // Door chime
      case 'r': case 'R': dscPanelKey = 0xDA; break;                                             // Reset
      case 'p': case 'P': dscPanelKey = 0xDD; dscWriteAlarm = true; break;                       // Keypad panic alarm
      case 'x': case 'X': dscPanelKey = 0xE1; break;                                             // Exit
      case '[': dscPanelKey = 0xD5; dscWriteArm[dscWritePartition - 1] = true; break;            // Command output 1
      case ']': dscPanelKey = 0xDA; dscWriteArm[dscWritePartition - 1] = true; break;            // Command output 2
      case '{': dscPanelKey = 0x70; dscWriteArm[dscWritePartition - 1] = true; break;            // Command output 3
      case '}': dscPanelKey = 0xEC; dscWriteArm[dscWritePartition - 1] = true; break;            // Command output 4
      default: {
        validKey = false;
        break;
      }
    }

    // Sets the writing position in dscClockInterrupt() for the currently set partition
    if (dscPartitions < dscWritePartition) dscWritePartition = 1;
    switch (dscWritePartition) {
      case 1:
      case 5: {
        dscWriteByte = 2;
        dscWriteBit = 9;
        break;
      }
      case 2:
      case 6: {
        dscWriteByte = 3;
        dscWriteBit = 17;
        break;
      }
      case 3:
      case 7: {
        dscWriteByte = 8;
        dscWriteBit = 57;
        break;
      }
      case 4:
      case 8: {
        dscWriteByte = 9;
        dscWriteBit = 65;
        break;
      }
      default: {
        dscWriteByte = 2;
        dscWriteBit = 9;
        break;
      }
    }

    if (dscWriteAlarm) previousTime = esp_timer_get_time();  // Sets a marker to time writes after keypad alarm keys
    if (validKey) dscPanelKeyPending = true;     // Sets a flag indicating that a write is pending, cleared by dscClockInterrupt()
  }
}


void dscSetup() {

  // Timer setup
  timer_config_t timerConfig = {
      .alarm_en = TIMER_ALARM_EN,
      .counter_en = TIMER_PAUSE,
      .intr_type = TIMER_INTR_LEVEL,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = TIMER_AUTORELOAD_EN,
      .divider = dscTimerDivider
  };
  timer_init(dscTimerGroup, dscTimerNumber, &timerConfig);
  timer_set_counter_value(dscTimerGroup, dscTimerNumber, 0);
  timer_set_alarm_value(dscTimerGroup, dscTimerNumber, dscTimerInterval);
  timer_enable_intr(dscTimerGroup, dscTimerNumber);
  timer_isr_callback_add(dscTimerGroup, dscTimerNumber, dscDataInterrupt, NULL, dscTimerISRPriority);

  // GPIO setup
  gpio_config_t gpioConfig = {};
  gpioConfig.intr_type = GPIO_INTR_DISABLE;
  gpioConfig.mode = GPIO_MODE_OUTPUT;
  gpioConfig.pin_bit_mask = dscWritePinMask;
  gpioConfig.pull_down_en = 0;
  gpioConfig.pull_up_en = 0;
  gpio_config(&gpioConfig);

  gpioConfig.pin_bit_mask = dscReadPinMask;
  gpioConfig.mode = GPIO_MODE_INPUT;
  gpio_config(&gpioConfig);

  gpioConfig.pin_bit_mask = dscClockPinMask;
  gpioConfig.intr_type = GPIO_INTR_ANYEDGE;
  gpioConfig.mode = GPIO_MODE_INPUT;
  gpio_config(&gpioConfig);

  #ifdef GPIODEBUG
  gpio_set_level(dscLogicPin0, 0);
  gpio_set_level(dscLogicPin1, 0);
  #endif

  gpio_install_isr_service(dscGPIOISRPriority);
  gpio_isr_handler_add(dscClockPin, dscClockInterrupt, NULL);

  vTaskDelete(NULL);
}


void dscStop() {
  gpio_isr_handler_remove(dscClockPin);
  timer_disable_intr(dscTimerGroup, dscTimerNumber);

  // Resets the panel capture data and counters
  dscPanelBufferLength = 0;
  for (uint8_t i = 0; i < dscReadSize; i++) dscIsrPanelData[i] = 0;
  dscIsrPanelBitTotal = 0;
  dscIsrPanelBitCount = 0;
  dscIsrPanelByteCount = 0;
}


void dsc_init() {

  // Settings
  dscPanelKeyPending = false;
  dscWritePartition = 1;
  dscPauseStatus = false;

  // Task setup
  dscDataAvailable = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(dscSetup, "dscSetup", 4096, NULL, 0, NULL, APP_CPU_NUM);  // Ensures timer interrupt is run on the app core to minimize contention with WiFi interrupts
  xTaskCreatePinnedToCore(dscPanelLoop, "dscPanelLoop", 4096, NULL, 0, NULL, APP_CPU_NUM);
  xTaskCreatePinnedToCore(dscLoop, "dscLoop", 4096, NULL, 0, NULL, APP_CPU_NUM);
}
