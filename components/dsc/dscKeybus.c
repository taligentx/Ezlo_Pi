#include "dscKeybus.h"

// Handler for critical sections
static portMUX_TYPE dscSpinlock = portMUX_INITIALIZER_UNLOCKED;


// Processes new panel data
static void dscProcess(void *params) {

  // Sets this task to be notified by dscDataInterrupt() when new data is available
  dscProcessHandle = xTaskGetCurrentTaskHandle();

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
    static uint8_t panelBufferIndex = 1;
    uint8_t dataIndex = panelBufferIndex - 1;
    for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) dscPanelData[i] = dscPanelBuffer[dataIndex][i];
    dscPanelBitCount = dscPanelBufferBitCount[dataIndex];
    dscPanelByteCount = dscPanelBufferByteCount[dataIndex];
    panelBufferIndex++;

    // Resets counters when the buffer is cleared
    taskENTER_CRITICAL(&dscSpinlock);
    if (panelBufferIndex > dscPanelBufferLength) {
      panelBufferIndex = 1;
      dscPanelBufferLength = 0;
    }
    taskEXIT_CRITICAL(&dscSpinlock);

    // Waits at startup for the 0x05 or 0x1B status command or a command with valid CRC data to eliminate spurious data.
    static bool startupCycle = true;
    if (startupCycle) {
      if (dscPanelData[0] == 0) continue;
      else if (dscPanelData[0] == 0x05 || dscPanelData[0] == 0x1B) {
        if (dscPanelByteCount == 6) dscKeybusVersion1 = true;
        startupCycle = false;
        dscWriteReady = true;
      }
      else if (!dscValidCRC()) continue;
    }

    // Sets dscWriteReady status
    taskENTER_CRITICAL(&dscSpinlock);
    if (!dscWriteKeyPending && !dscWriteKeysPending) dscWriteReady = true;
    else dscWriteReady = false;
    taskEXIT_CRITICAL(&dscSpinlock);

    // Skips redundant data sent constantly while in installer programming
    static uint8_t previousCmd0A[DSC_DATA_SIZE], previousCmd0F[DSC_DATA_SIZE], previousCmdE6_20[DSC_DATA_SIZE], previousCmdE6_21[DSC_DATA_SIZE];
    switch (dscPanelData[0]) {
      case 0x0A:  // Partition 1 status in programming
        if (dscRedundantPanelData(previousCmd0A, dscPanelData, DSC_DATA_SIZE)) continue;
        break;

      case 0x0F:  // Partition 2 status in programming
        if (dscRedundantPanelData(previousCmd0F, dscPanelData, DSC_DATA_SIZE)) continue;
        break;

      case 0xE6:
        if (dscPanelData[2] == 0x20 && dscRedundantPanelData(previousCmdE6_20, dscPanelData, DSC_DATA_SIZE)) continue;  // Partition 1 status in programming, zone lights 33-64
        if (dscPanelData[2] == 0x21 && dscRedundantPanelData(previousCmdE6_21, dscPanelData, DSC_DATA_SIZE)) continue;  // Partition 2 status in programming, zone lights 33-64
        break;
    }
    if (DSC_PARTITIONS > 4) {
      static uint8_t previousCmdE6_03[DSC_DATA_SIZE];
      if (dscPanelData[0] == 0xE6 && dscPanelData[2] == 0x03 && dscRedundantPanelData(previousCmdE6_03, dscPanelData, 8)) continue;  // Status in alarm/programming, partitions 5-8
    }

    // Processes valid panel data
    switch (dscPanelData[0]) {
      case 0x05:                                                        // Panel status: partitions 1-4
      case 0x1B: dscProcessPanelStatus(); break;                        // Panel status: partitions 5-8
      case 0x16: dscProcessPanel_0x16(); break;                         // Panel configuration
      case 0x27: dscProcessPanel_0x27(); break;                         // Panel status and zones 1-8 status
      case 0x2D: dscProcessPanel_0x2D(); break;                         // Panel status and zones 9-16 status
      case 0x34: dscProcessPanel_0x34(); break;                         // Panel status and zones 17-24 status
      case 0x3E: dscProcessPanel_0x3E(); break;                         // Panel status and zones 25-32 status
      case 0x87: dscProcessPanel_0x87(); break;                         // PGM outputs
      case 0xA5: dscProcessPanel_0xA5(); break;                         // Date, time, system status messages - partitions 1-2
      case 0xE6: if (DSC_PARTITIONS > 2) dscProcessPanel_0xE6(); break;  // Extended status command split into multiple subcommands to handle up to 8 partitions/64 zones
      case 0xEB: if (DSC_PARTITIONS > 2) dscProcessPanel_0xEB(); break;  // Date, time, system status messages - partitions 1-8
    }

    dscPanelDataAvailable = true;
    xSemaphoreGive(dscDataAvailable);
  }
}


// Sets up writes for a single key, blocks if a previous write is in progress
void dscWriteKey(int receivedKey) {
  while (dscWriteKeyPending || dscWriteKeysPending) vTaskDelay(pdMS_TO_TICKS(1));
  dscSetWriteKey(receivedKey);
}


// Writes multiple keys sent as a char array - blocks until the write is complete
void dscWriteKeys(const char *receivedKeys) {

  // Blocks if a previous write is in progress
  while (dscWriteKeyPending || dscWriteKeysPending) vTaskDelay(pdMS_TO_TICKS(1));

  if (receivedKeys[0] != '\0') {
    dscWriteKeysPending = true;
    dscWriteReady = false;
  }

  while (dscWriteKeysPending) {
    static uint8_t writeCounter = 0;
    if (!dscWriteKeyPending && dscWriteKeysPending && writeCounter < strlen(receivedKeys)) {
      if (receivedKeys[writeCounter] != '\0') {
        dscSetWriteKey(receivedKeys[writeCounter]);
        writeCounter++;
        if (receivedKeys[writeCounter] == '\0') {
          dscWriteKeysPending = false;
          writeCounter = 0;
        }
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1));
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
  if (!dscWriteKeyPending && (esp_timer_get_time() - previousTime > 500000 || esp_timer_get_time() <= 500000)) {
    bool validKey = true;

    // Skips writing to disabled partitions or partitions not specified in dscKeybus.h
    if (dscDisabled[dscWritePartition - 1] || DSC_PARTITIONS < dscWritePartition) {
      switch (receivedKey) {
        case '/': setPartition = true; validKey = false; break;
      }
      return;
    }

    // Sets binary for virtual keypad keys
    else {
      switch (receivedKey) {
        case '/': setPartition = true; validKey = false; break;
        case '0': dscWriteKeyData = 0x00; break;
        case '1': dscWriteKeyData = 0x05; break;
        case '2': dscWriteKeyData = 0x0A; break;
        case '3': dscWriteKeyData = 0x0F; break;
        case '4': dscWriteKeyData = 0x11; break;
        case '5': dscWriteKeyData = 0x16; break;
        case '6': dscWriteKeyData = 0x1B; break;
        case '7': dscWriteKeyData = 0x1C; break;
        case '8': dscWriteKeyData = 0x22; break;
        case '9': dscWriteKeyData = 0x27; break;
        case '*': dscWriteKeyData = 0x28; if (dscStatus[dscWritePartition - 1] < 0x9E) dscStarKeyCheck = true; break;
        case '#': dscWriteKeyData = 0x2D; break;
        case 'f': case 'F': dscWriteKeyData = 0xBB; dscWriteAlarm = true; break;                              // Keypad fire alarm
        case 'b': case 'B': dscWriteKeyData = 0x82; break;                                                    // Enter event buffer
        case '>': dscWriteKeyData = 0x87; break;                                                              // Event buffer right arrow
        case '<': dscWriteKeyData = 0x88; break;                                                              // Event buffer left arrow
        case 'l': case 'L': dscWriteKeyData = 0xA5; break;                                                    // LCD keypad data request
        case 's': case 'S': dscWriteKeyData = 0xAF; dscWriteAccessCode[dscWritePartition - 1] = true; break;  // Arm stay
        case 'w': case 'W': dscWriteKeyData = 0xB1; dscWriteAccessCode[dscWritePartition - 1] = true; break;  // Arm away
        case 'n': case 'N': dscWriteKeyData = 0xB6; dscWriteAccessCode[dscWritePartition - 1] = true; break;  // Arm with no entry delay (night arm)
        case 'a': case 'A': dscWriteKeyData = 0xDD; dscWriteAlarm = true; break;                              // Keypad auxiliary alarm
        case 'c': case 'C': dscWriteKeyData = 0xBB; break;                                                    // Door chime
        case 'r': case 'R': dscWriteKeyData = 0xDA; break;                                                    // Reset
        case 'p': case 'P': dscWriteKeyData = 0xEE; dscWriteAlarm = true; break;                              // Keypad panic alarm
        case 'x': case 'X': dscWriteKeyData = 0xE1; break;                                                    // Exit
        case '[': dscWriteKeyData = 0xD5; dscWriteAccessCode[dscWritePartition - 1] = true; break;            // Command output 1
        case ']': dscWriteKeyData = 0xDA; dscWriteAccessCode[dscWritePartition - 1] = true; break;            // Command output 2
        case '{': dscWriteKeyData = 0x70; dscWriteAccessCode[dscWritePartition - 1] = true; break;            // Command output 3
        case '}': dscWriteKeyData = 0xEC; dscWriteAccessCode[dscWritePartition - 1] = true; break;            // Command output 4
        default: {
          validKey = false;
          break;
        }
      }
    }

    // Sets the writing position in dscClockInterrupt() for the currently set partition
    switch (dscWritePartition) {
      case 1: dscWriteCommand = 0x05; dscWriteByte = 2; break;
      case 2: dscWriteCommand = 0x05; dscWriteByte = 3; break;
      case 3: dscWriteCommand = 0x05; dscWriteByte = 8; break;
      case 4: dscWriteCommand = 0x05; dscWriteByte = 9; break;
      case 5: dscWriteCommand = 0x1B; dscWriteByte = 2; break;
      case 6: dscWriteCommand = 0x1B; dscWriteByte = 3; break;
      case 7: dscWriteCommand = 0x1B; dscWriteByte = 8; break;
      case 8: dscWriteCommand = 0x1B; dscWriteByte = 9; break;
      default: dscWriteCommand = 0x05; dscWriteByte = 2;
    }

    if (dscWriteAlarm) previousTime = esp_timer_get_time();  // Sets a marker to time writes after keypad alarm keys
    if (validKey) {
      dscWriteKeyPending = true;                             // Sets a flag indicating that a write is pending, cleared by dscClockInterrupt()
      dscWriteReady = false;
    }
  }
}


// Checks for redundant panel data
bool dscRedundantPanelData(uint8_t previousCmd[], volatile uint8_t currentCmd[], uint8_t checkedBytes) {
  bool redundantData = true;
  for (uint8_t i = 0; i < checkedBytes; i++) {
    if (previousCmd[i] != currentCmd[i]) {
      redundantData = false;
      break;
    }
  }
  if (redundantData) return true;
  else {
    for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) previousCmd[i] = currentCmd[i];
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


// Checks panel data CRC
bool IRAM_ATTR dscValidCRC(void) {
  uint8_t byteCount = (dscPanelBitCount - 1) / 8;
  int dataSum = 0;
  for (uint8_t panelByte = 0; panelByte < byteCount; panelByte++) {
    if (panelByte != 1) dataSum += dscPanelData[panelByte];
  }
  if (dataSum % 256 == dscPanelData[byteCount]) return true;
  else return false;
}


// Called as an interrupt when the DSC clock changes to write data for virtual keypad
// and setup timers to read data after an interval.
static void IRAM_ATTR dscClockInterrupt(void *params) {

  // Sets up a timer that will call dscDataInterrupt() after DSC_TIMER_INTERVAL to read the data line. Data sent
  // from the panel and keypads/modules has latency after a clock change (observed up to 160us for keypad data).
  timer_group_set_counter_enable_in_isr(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, TIMER_START);

  static int64_t previousClockHighTime;

  // Panel sends data when the clock is high
  if (dscGetLevelISR(DSC_CLOCK_PIN) == 1) {
    dscSetLevelISR(DSC_WRITE_PIN, 0);  // Restores the data line after a virtual keypad write
    previousClockHighTime = esp_timer_get_time();
    GPIO.pin[DSC_CLOCK_PIN].int_type = GPIO_INTR_LOW_LEVEL;  // Toggles the interrupt type
    return;
  }

  // Keypads and modules send data while the clock is low
  dscClockHighTime = esp_timer_get_time() - previousClockHighTime;  // Tracks the clock high time to find the reset between commands

  GPIO.pin[DSC_CLOCK_PIN].int_type = GPIO_INTR_HIGH_LEVEL;  // Toggles the interrupt type

  static bool writeStart = false;
  static bool wroteKey = false;

  // Saves data and resets counters after the clock cycle is complete (high for at least 1ms)
  if (dscClockHighTime > 1000) {
    dscKeybusTime = esp_timer_get_time();
    bool skipData = false;

    // Skips incomplete data
    if (dscIsrPanelBitTotal < 8) skipData = true;

    // Skips redundant 0x05 and 0x1B status data to prevent flooding the buffer as these are sent
    // constantly at a high rate and verifies new status data by requiring DSC_VERIFY_COUNT
    // identical commands as these do not contain a CRC.
    else {
      uint8_t index;
      bool redundantData = true;
      switch (dscIsrPanelData[0]) {
        case 0x05: index = 0; break;  // Status: partitions 1-4
        case 0x1B: index = 1; break;  // Status: partitions 5-8
        default: redundantData = false; break;
      }

      if (redundantData) {
        static uint8_t previousCmd[2][DSC_DATA_SIZE];
        static uint8_t pendingCmd[2][DSC_DATA_SIZE];
        static uint8_t countCmd[2];

        for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) {
          if ((previousCmd[index][i] != dscIsrPanelData[i]) || countCmd[index] > 0) {

            if (DSC_VERIFY_COUNT <= 1) redundantData = false;

            else if (countCmd[index] == 0) {  // New pending data
              for (uint8_t j = 0; j < DSC_DATA_SIZE; j++) pendingCmd[index][j] = dscIsrPanelData[j];
              countCmd[index]++;
            }

            else {
              bool newData = false;
              for (uint8_t j = 0; j < DSC_DATA_SIZE; j++) {  // Checks if new data against pending data
                if (pendingCmd[index][j] != dscIsrPanelData[j]) {
                  newData = true;
                  break;
                }
              }

              if (newData) {  // Replaces pending data with new data
                for (uint8_t j = 0; j < DSC_DATA_SIZE; j++) pendingCmd[index][j] = dscIsrPanelData[j];
                countCmd[index] = 0;
              }

              else if (countCmd[index] >= (DSC_VERIFY_COUNT - 1)) {  // Sets data as verified
                redundantData = false;
                countCmd[index] = 0;
              }

              else countCmd[index]++;  // New data matches pending data, increment verification counter
            }
            break;
          }
        }

        if (redundantData) skipData = true;
        else {
          for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) previousCmd[index][i] = dscIsrPanelData[i];
          skipData = false;
        }
      }
    }

    // Stores new panel data in the panel buffer
    if (dscPanelBufferLength == DSC_BUFFER_SIZE) dscBufferOverflow = true;

    else if (!skipData && dscPanelBufferLength < DSC_BUFFER_SIZE) {
      for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) dscPanelBuffer[dscPanelBufferLength][i] = dscIsrPanelData[i];
      dscPanelBufferBitCount[dscPanelBufferLength] = dscIsrPanelBitTotal;
      dscPanelBufferByteCount[dscPanelBufferLength] = dscIsrPanelByteCount;
      dscPanelBufferLength++;
    }

    // Resets the panel capture data and counters
    for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) dscIsrPanelData[i] = 0;
    dscIsrPanelBitTotal = 0;
    dscIsrPanelBitCount = 0;
    dscIsrPanelByteCount = 0;
    skipData = false;
    writeStart = false;
    if (wroteKey) {
      wroteKey = false;
      dscWriteCommand = 0xFF;
      if (dscStarKeyCheck) dscStarKeyWait[dscWritePartition - 1] = true;  // Handles waiting until the panel is ready after pressing '*'
      else dscWriteKeyPending = false;
    }

    // Notifies the dscProcess task when new data is available
    if (dscPanelBufferLength > 0) {
      BaseType_t xHigherPriorityTaskWoken;
      vTaskNotifyGiveFromISR(dscProcessHandle, &xHigherPriorityTaskWoken);
      if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
    }
  }

  // Increments the total bit count when the clock completes a HIGH to LOW cycle
  else dscIsrPanelBitTotal++;

  // Virtual keypad writes to the panel while the clock is low
  static uint8_t writeDataIndex = 0;
  static bool writeRepeat = false;

  // Writes a keypad F/A/P alarm and repeats the key on the next immediate command from the panel (0x1C verification)
  if ((dscWriteAlarm && dscWriteKeyPending) || writeRepeat) {

    // Writes the first bit by shifting the alarm key data right 7 bits and checking bit 0
    if (dscIsrPanelBitTotal == 0) {

      if (!((dscWriteKeyData >> 7) & 0x01)) {
        dscSetLevelISR(DSC_WRITE_PIN, 1);
      }
      writeStart = true;  // Resolves a timing issue where some writes do not begin at the correct bit
    }

    // Writes the remaining alarm key data
    else if (writeStart && dscIsrPanelBitTotal <= 7) {
      if (!((dscWriteKeyData >> (7 - dscIsrPanelBitTotal)) & 0x01)) {
        dscSetLevelISR(DSC_WRITE_PIN, 1);
      }

      // Resets counters when the write is complete
      if (dscIsrPanelBitTotal == 7) {
        dscWriteKeyPending = false;
        writeStart = false;
        dscWriteAlarm = false;

        // Sets up a repeated write for alarm keys
        if (!writeRepeat) writeRepeat = true;
        else writeRepeat = false;
      }
    }
  }

  // Writes all other data during panel commands
  else {

    // Checks dscWriteData[] for data to write associated with the current panel command
    if (dscIsrPanelBitTotal == 8) {
      for (uint8_t i = 0; i < DSC_WRITE_SIZE; i++) {
        if (dscWriteData[i][0] == dscIsrPanelData[0]) {
          writeDataIndex = i;
          writeStart = true;

          // Checks for keypad keys to write and copies the key to dscWriteData[]
          if (dscWriteKeyPending && !dscStarKeyWait[dscWritePartition - 1]) {
            if (dscWriteCommand == dscIsrPanelData[0]) {
              dscWriteData[i][dscWriteByte] = dscWriteKeyData;
            }
          }
          break;
        }
      }
    }

    // Writes data from dscWriteData[] to the panel
    if (writeStart) {
      if (dscWriteData[writeDataIndex][dscIsrPanelByteCount] != 0xFF) {

        // Writes data
        if (!bitRead(dscWriteData[writeDataIndex][dscIsrPanelByteCount], (7 - dscIsrPanelBitCount))) {
          dscSetLevelISR(DSC_WRITE_PIN, 1);
        }

        // Handles keypad key writes
        if (dscWriteData[writeDataIndex][0] == dscWriteCommand) {

          // Sets a flag to reset dscWriteKeyPending at the end of the panel command
          if (dscIsrPanelByteCount == dscWriteByte) wroteKey = true;

          // Resets write data for 0x05 and 0x1B - these are one-time writes (keypad keys, module status change notifications, etc)
          if (dscIsrPanelBitCount == 7) dscWriteData[writeDataIndex][dscIsrPanelByteCount] = 0xFF;
        }
      }
    }
  }
}


// Timer interrupt called by dscClockInterrupt() after DSC_TIMER_INTERVAL to read the data line
// Data sent from the panel and keypads/modules has latency after a clock change (observed up to 160us for keypad data).
static bool IRAM_ATTR dscDataInterrupt(void *params) {
  timer_group_set_counter_enable_in_isr(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, TIMER_PAUSE);

  // Panel sends data while the clock is high
  if (dscGetLevelISR(DSC_CLOCK_PIN) == 1) {

    // Reads panel data and sets data counters
    if (dscIsrPanelByteCount < DSC_DATA_SIZE) {  // Limits Keybus data bytes to DSC_DATA_SIZE
      if (dscIsrPanelBitCount < 8) {

        // Data is captured in each byte by shifting left by 1 bit and writing to bit 0
        dscIsrPanelData[dscIsrPanelByteCount] <<= 1;
        if (dscGetLevelISR(DSC_READ_PIN) == 1) {
          dscIsrPanelData[dscIsrPanelByteCount] |= 1;
        }
      }

      // Stores the stop bit by itself in byte 1 - this aligns the Keybus bytes with dscPanelData[] bytes
      if (dscIsrPanelBitTotal == 8) {
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
    }
  }

  // Return false to indicate no need to task yield
  return false;
}


static void dscSetup(void *params) {

  // Timer setup
  timer_config_t timerConfig = {
      .alarm_en = TIMER_ALARM_EN,
      .counter_en = TIMER_PAUSE,
      .intr_type = TIMER_INTR_LEVEL,
      .counter_dir = TIMER_COUNT_UP,
      .auto_reload = TIMER_AUTORELOAD_EN,
      .divider = DSC_TIMER_DIVIDER
  };
  timer_init(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, &timerConfig);
  timer_set_counter_value(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, 0);
  timer_set_alarm_value(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, DSC_TIMER_INTERVAL);
  timer_enable_intr(DSC_TIMER_GROUP, DSC_TIMER_NUMBER);
  timer_isr_callback_add(DSC_TIMER_GROUP, DSC_TIMER_NUMBER, dscDataInterrupt, NULL, DSC_TIMER_ISR_PRIORITY);

  // GPIO setup
  gpio_config_t gpioConfig = {};
  gpioConfig.intr_type = GPIO_INTR_DISABLE;
  gpioConfig.mode = GPIO_MODE_OUTPUT;
  gpioConfig.pin_bit_mask = BIT64(DSC_WRITE_PIN);
  gpioConfig.pull_down_en = 0;
  gpioConfig.pull_up_en = 0;
  gpio_config(&gpioConfig);

  gpioConfig.pin_bit_mask = BIT64(DSC_READ_PIN);
  gpioConfig.mode = GPIO_MODE_INPUT;
  gpio_config(&gpioConfig);

  gpioConfig.pin_bit_mask = BIT64(DSC_CLOCK_PIN);
  gpioConfig.intr_type = GPIO_INTR_HIGH_LEVEL;
  gpioConfig.mode = GPIO_MODE_INPUT;
  gpio_config(&gpioConfig);

  gpio_install_isr_service(DSC_GPIO_ISR_PRIORITY);
  gpio_isr_handler_add(DSC_CLOCK_PIN, dscClockInterrupt, NULL);

  vTaskDelete(NULL);
}


void dsc_stop(void) {
  gpio_isr_handler_remove(DSC_CLOCK_PIN);
  timer_disable_intr(DSC_TIMER_GROUP, DSC_TIMER_NUMBER);

  // Resets the panel capture data and counters
  dscPanelBufferLength = 0;
  for (uint8_t i = 0; i < DSC_DATA_SIZE; i++) dscIsrPanelData[i] = 0;
  dscIsrPanelBitTotal = 0;
  dscIsrPanelBitCount = 0;
  dscIsrPanelByteCount = 0;
}


void dsc_init(void) {

  // Settings
  dscWriteReady = false;
  dscWritePartition = 1;
  dscPauseStatus = false;
  for (uint8_t i = 0; i < DSC_WRITE_SIZE; i++) {
    for (uint8_t j = 0; j < DSC_DATA_SIZE; j++) {
      dscWriteData[i][j] = 0xFF;
    }
  }
  dscWriteData[0][0] = 0x05;
  if (DSC_WRITE_SIZE > 1) dscWriteData[1][0] = 0x11;
  if (DSC_PARTITIONS > 4 && dscWriteData[DSC_WRITE_SIZE - 1][0] == 0xFF) dscWriteData[DSC_WRITE_SIZE - 1][0] = 0x1B;

  // Task setup
  dscDataAvailable = xSemaphoreCreateBinary();
  xTaskCreatePinnedToCore(dscSetup, "dscSetup", 4 * 1024, NULL, 0, NULL, APP_CPU_NUM);  // Ensures GPIO and timer interrupts are run on the app core to minimize contention with WiFi/BLE interrupts
  xTaskCreate(dscProcess, "dscProcess", 4 * 1024, NULL, 0, NULL);
  xTaskCreate(dscExample, "dscExample", 4 * 1024, NULL, 0, NULL);
}
