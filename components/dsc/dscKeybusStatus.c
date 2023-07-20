#include "dscKeybus.h"


// Resets the state of all status components as changed for programs to get the current status
void dsc_reset() {
  dscStatusChanged = true;
  dscKeybusChanged = true;
  dscTroubleChanged = true;
  dscPowerChanged = true;
  dscBatteryChanged = true;
  for (uint8_t partition = 0; partition < DSC_PARTITIONS; partition++) {
    dscReadyChanged[partition] = true;
    dscArmedChanged[partition] = true;
    dscAlarmChanged[partition] = true;
    dscFireChanged[partition] = true;
    dscDisabled[partition] = true;
  }
  dscOpenZonesStatusChanged = true;
  dscAlarmZonesStatusChanged = true;
  for (uint8_t zoneGroup = 0; zoneGroup < DSC_ZONES; zoneGroup++) {
    dscOpenZonesChanged[zoneGroup] = 0xFF;
    dscAlarmZonesChanged[zoneGroup] = 0xFF;
  }
  dscPgmOutputsChanged[0] = 0xFF;
  dscPgmOutputsChanged[1] = 0x3F;
}


// Sets the panel time
bool dscSetTime(unsigned int year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, const char* dscAccessCode, uint8_t timePartition) {

  if (!dscReady[0]) return false;  // Skips if partition 1 is not ready

  // Loops if a previous write is in progress
  while (dscWriteKeyPending || dscWriteKeysPending) vTaskDelay(pdMS_TO_TICKS(1));

  if (hour > 23 || minute > 59 || month > 12 || day > 31 || year > 2099 || (year > 99 && year < 1900)) return false;  // Skips if input date/time is invalid
  static char timeEntry[21];
  strcpy(timeEntry, "*6");
  strcat(timeEntry, dscAccessCode);
  strcat(timeEntry, "1");

  char timeChar[3];
  if (hour < 10) strcat(timeEntry, "0");
  itoa(hour, timeChar, 10);
  strcat(timeEntry, timeChar);

  if (minute < 10) strcat(timeEntry, "0");
  itoa(minute, timeChar, 10);
  strcat(timeEntry, timeChar);

  if (month < 10) strcat(timeEntry, "0");
  itoa(month, timeChar, 10);
  strcat(timeEntry, timeChar);

  if (day < 10) strcat(timeEntry, "0");
  itoa(day, timeChar, 10);
  strcat(timeEntry, timeChar);

  if (year >= 2000) year -= 2000;
  else if (year >= 1900) year -= 1900;
  if (year < 10) strcat(timeEntry, "0");
  itoa(year, timeChar, 10);
  strcat(timeEntry, timeChar);

  strcat(timeEntry, "#");

  if (dscWritePartition != timePartition) {
    uint8_t previousPartition = dscWritePartition;
    dscWritePartition = timePartition;
    dscWriteKeys(timeEntry);
    while (dscWriteKeyPending || dscWriteKeysPending) vTaskDelay(pdMS_TO_TICKS(1));
    dscWritePartition = previousPartition;
  }
  else dscWriteKeys(timeEntry);

  return true;
}


// Processes status commands: 0x05 (Partitions 1-4) and 0x1B (Partitions 5-8)
void dscProcessPanelStatus() {

  // Trouble status
  if (dscPanelData[3] <= 0x06) {  // Ignores trouble light status in intermittent states
    if (bitRead(dscPanelData[2],4)) dscTrouble = true;
    else dscTrouble = false;
    if (dscTrouble != dscPreviousTrouble) {  // Ignores trouble light status in intermittent states
      dscPreviousTrouble = dscTrouble;
      dscTroubleChanged = true;
      if (!dscPauseStatus) dscStatusChanged = true;
    }
  }

  uint8_t partitionStart = 0;
  uint8_t partitionCount = 0;
  if (dscPanelData[0] == 0x05) {
    partitionStart = 0;
    if (dscKeybusVersion1) partitionCount = 2;  // Handles earlier panels that support up to 2 partitions
    else partitionCount = 4;
    if (DSC_PARTITIONS < partitionCount) partitionCount = DSC_PARTITIONS;
  }
  else if (DSC_PARTITIONS > 4 && dscPanelData[0] == 0x1B) {
    partitionStart = 4;
    partitionCount = 8;
    if (DSC_PARTITIONS < partitionCount) partitionCount = DSC_PARTITIONS;
  }

  // Sets status per partition
  for (uint8_t partitionIndex = partitionStart; partitionIndex < partitionCount; partitionIndex++) {
    uint8_t statusByte, messageByte;
    if (partitionIndex < 4) {
      statusByte = (partitionIndex * 2) + 2;
      messageByte = (partitionIndex * 2) + 3;
    }
    else {
      statusByte = ((partitionIndex - 4) * 2) + 2;
      messageByte = ((partitionIndex - 4) * 2) + 3;
    }

    // Partition disabled status
    if (dscPanelData[messageByte] == 0xC7) {
      dscDisabled[partitionIndex] = true;
      dscProcessReadyStatus(partitionIndex, false);
    }
    else dscDisabled[partitionIndex] = false;
    if (dscDisabled[partitionIndex] != dscPreviousDisabled[partitionIndex]) {
      dscPreviousDisabled[partitionIndex] = dscDisabled[partitionIndex];
      dscDisabledChanged[partitionIndex] = true;
      if (!dscPauseStatus) dscStatusChanged = true;
    }

    // Status lights
    dscLights[partitionIndex] = dscPanelData[statusByte];
    if (dscLights[partitionIndex] != dscPreviousLights[partitionIndex]) {
      dscPreviousLights[partitionIndex] = dscLights[partitionIndex];
      if (!dscPauseStatus) dscStatusChanged = true;
    }

    // Status messages
    dscStatus[partitionIndex] = dscPanelData[messageByte];
    if (dscStatus[partitionIndex] != dscPreviousStatus[partitionIndex]) {
      dscPreviousStatus[partitionIndex] = dscStatus[partitionIndex];
      if (!dscPauseStatus) dscStatusChanged = true;
    }

    // Fire status
    if (dscPanelData[messageByte] < 0x12) {  // Ignores fire light status in intermittent states
      if (bitRead(dscPanelData[statusByte],6)) dscFire[partitionIndex] = true;
      else dscFire[partitionIndex] = false;
      if (dscFire[partitionIndex] != dscPreviousFire[partitionIndex]) {
        dscPreviousFire[partitionIndex] = dscFire[partitionIndex];
        dscFireChanged[partitionIndex] = true;
        if (!dscPauseStatus) dscStatusChanged = true;
      }
    }

    // Messages
    switch (dscPanelData[messageByte]) {

      // Ready
      case 0x01:         // Partition ready
      case 0x02: {       // Stay/away zones open
        dscProcessReadyStatus(partitionIndex, true);
        dscProcessEntryDelayStatus(partitionIndex, false);

        dscArmedStay[partitionIndex] = false;
        dscArmedAway[partitionIndex] = false;
        dscArmed[partitionIndex] = false;
        if (dscArmed[partitionIndex] != dscPreviousArmed[partitionIndex]) {
          dscPreviousArmed[partitionIndex] = dscArmed[partitionIndex];
          dscArmedChanged[partitionIndex] = true;
          if (!dscPauseStatus) dscStatusChanged = true;
        }

        dscProcessAlarmStatus(partitionIndex, false);
        break;
      }

      // Zones open
      case 0x03: {
        dscProcessReadyStatus(partitionIndex, false);
        dscProcessEntryDelayStatus(partitionIndex, false);
        break;
      }

      // Armed
      case 0x04:         // Armed stay
      case 0x05: {       // Armed away
        if (dscPanelData[messageByte] == 0x04) {
          dscArmedStay[partitionIndex] = true;
          dscArmedAway[partitionIndex] = false;
        }
        else {
          dscArmedStay[partitionIndex] = false;
          dscArmedAway[partitionIndex] = true;
        }

        dscWriteAccessCode[partitionIndex] = false;

        dscArmed[partitionIndex] = true;
        if (dscArmed[partitionIndex] != dscPreviousArmed[partitionIndex] || dscArmedStay[partitionIndex] != dscPreviousArmedStay[partitionIndex]) {
          dscPreviousArmed[partitionIndex] = dscArmed[partitionIndex];
          dscPreviousArmedStay[partitionIndex] = dscArmedStay[partitionIndex];
          dscArmedChanged[partitionIndex] = true;
          if (!dscPauseStatus) dscStatusChanged = true;
        }

        dscProcessReadyStatus(partitionIndex, false);
        dscProcessExitDelayStatus(partitionIndex, false);
        dscExitState[partitionIndex] = DSC_EXIT_NONE;
        dscProcessEntryDelayStatus(partitionIndex, false);
        break;
      }

      // Exit delay in progress
      case 0x08: {
        dscWriteAccessCode[partitionIndex] = false;

        dscProcessExitDelayStatus(partitionIndex, true);

        if (dscExitState[partitionIndex] != DSC_EXIT_NO_ENTRY_DELAY) {
          if (bitRead(dscLights[partitionIndex],3)) dscExitState[partitionIndex] = DSC_EXIT_STAY;
          else dscExitState[partitionIndex] = DSC_EXIT_AWAY;
          if (dscExitState[partitionIndex] != dscPreviousExitState[partitionIndex]) {
            dscPreviousExitState[partitionIndex] = dscExitState[partitionIndex];
            dscExitDelayChanged[partitionIndex] = true;
            dscExitStateChanged[partitionIndex] = true;
            if (!dscPauseStatus) dscStatusChanged = true;
          }
        }

        dscProcessReadyStatus(partitionIndex, true);
        break;
      }

      // Arming with no entry delay
      case 0x09: {
        dscProcessReadyStatus(partitionIndex, true);
        dscExitState[partitionIndex] = DSC_EXIT_NO_ENTRY_DELAY;
        break;
      }

      // Entry delay in progress
      case 0x0C: {
        dscProcessReadyStatus(partitionIndex, false);
        dscProcessEntryDelayStatus(partitionIndex, true);
        break;
      }

      // Partition in alarm
      case 0x11: {
        dscProcessReadyStatus(partitionIndex, false);
        dscProcessEntryDelayStatus(partitionIndex, false);
        dscProcessAlarmStatus(partitionIndex, true);
        break;
      }

      // Arming with bypassed zones
      case 0x15: {
        dscProcessReadyStatus(partitionIndex, true);
        break;
      }

      // Partition armed with no entry delay
      case 0x06:
      case 0x16: {
        dscArmed[partitionIndex] = true;

        // Sets an armed mode if not already set, used if interface is initialized while the panel is armed
        if (!dscArmedStay[partitionIndex] && !dscArmedAway[partitionIndex]) {
          if (dscPanelData[messageByte] == 0x06) {
            dscArmedStay[partitionIndex] = true;
            dscPreviousArmedStay[partitionIndex] = dscArmedStay[partitionIndex];
          }
          else dscArmedAway[partitionIndex] = true;
        }

        dscProcessNoEntryDelayStatus(partitionIndex, true);
        dscProcessReadyStatus(partitionIndex, false);
        break;
      }

      // Partition disarmed
      case 0x3D:
      case 0x3E: {
        if (dscPanelData[messageByte] == 0x3E) {  // Sets ready only during Partition disarmed
          dscProcessReadyStatus(partitionIndex, true);
        }

        dscProcessExitDelayStatus(partitionIndex, false);
        dscExitState[partitionIndex] = DSC_EXIT_NONE;
        dscProcessEntryDelayStatus(partitionIndex, false);
        dscProcessArmed(partitionIndex, false);
        dscProcessAlarmStatus(partitionIndex, false);
        break;
      }

      // Invalid access code
      case 0x8F: {
        if (!dscArmed[partitionIndex]) dscProcessReadyStatus(partitionIndex, true);
        break;
      }

      // Enter * function code
      case 0x9E:
      case 0xB8: {
        if (dscStarKeyWait[partitionIndex]) {  // Resets the flag that waits for panel status 0x9E, 0xB8 after '*' is pressed
          dscStarKeyWait[partitionIndex] = false;
          dscStarKeyCheck = false;
          dscWriteKeyPending = false;
        }
        dscProcessReadyStatus(partitionIndex, false);
        break;
      }

      // Enter access code
      case 0x9F: {
        if (dscWriteAccessCode[partitionIndex]) {  // Ensures access codes are only sent when an arm or command output key is sent through this interface
          dscWriteAccessCode[partitionIndex] = false;
          dscAccessCodePrompt = true;
          if (!dscPauseStatus) dscStatusChanged = true;
        }

        dscProcessReadyStatus(partitionIndex, false);
        break;
      }

      default: {
        dscReady[partitionIndex] = false;
        dscProcessReadyStatus(partitionIndex, false);
        break;
      }
    }
  }
}


void dscProcessPanel_0x16() {
  if (!dscValidCRC()) return;

  // Panel version
  dscPanelVersion = ((dscPanelData[3] >> 4) * 10) + (dscPanelData[3] & 0x0F);
}


// Panel status and zones 1-8 status
void dscProcessPanel_0x27() {
  if (!dscValidCRC()) return;

  // Messages
  for (uint8_t partitionIndex = 0; partitionIndex < 2; partitionIndex++) {
    uint8_t messageByte = (partitionIndex * 2) + 3;

    // Armed
    if (dscPanelData[messageByte] == 0x04 || dscPanelData[messageByte] == 0x05) {

      dscProcessReadyStatus(partitionIndex, false);

      if (dscPanelData[messageByte] == 0x04) {
        dscArmedStay[partitionIndex] = true;
        dscArmedAway[partitionIndex] = false;
      }
      else if (dscPanelData[messageByte] == 0x05) {
        dscArmedStay[partitionIndex] = false;
        dscArmedAway[partitionIndex] = true;
      }

      dscArmed[partitionIndex] = true;
      dscProcessExitDelayStatus(partitionIndex, false);

      dscExitState[partitionIndex] = DSC_EXIT_NONE;
    }

    // Armed with no entry delay
    else if (dscPanelData[messageByte] == 0x06 || dscPanelData[messageByte] == 0x16) {
      dscNoEntryDelay[partitionIndex] = true;

      // Sets an armed mode if not already set, used if interface is initialized while the panel is armed
      if (!dscArmedStay[partitionIndex] && !dscArmedAway[partitionIndex]) dscArmedStay[partitionIndex] = true;

      dscArmed[partitionIndex] = true;
      if (dscArmed[partitionIndex] != dscPreviousArmed[partitionIndex]) {
        dscPreviousArmed[partitionIndex] = dscArmed[partitionIndex];
        dscPreviousArmedStay[partitionIndex] = dscArmedStay[partitionIndex];
        dscArmedChanged[partitionIndex] = true;
        if (!dscPauseStatus) dscStatusChanged = true;
      }

      dscProcessExitDelayStatus(partitionIndex, false);
      dscExitState[partitionIndex] = DSC_EXIT_NONE;
      dscProcessReadyStatus(partitionIndex, false);
    }
  }

  // Zones 1-8 status is stored in dscOpenZones[0] and dscOpenZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
  dscProcessZoneStatus(0, 6);
}


// Zones 9-16 status is stored in dscOpenZones[1] and dscOpenZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
void dscProcessPanel_0x2D() {
  if (!dscValidCRC()) return;
  if (DSC_ZONES < 2) return;
  dscProcessZoneStatus(1, 6);
}


// Zones 17-24 status is stored in dscOpenZones[2] and dscOpenZonesChanged[2]: Bit 0 = Zone 17 ... Bit 7 = Zone 24
void dscProcessPanel_0x34() {
  if (!dscValidCRC()) return;
  if (DSC_ZONES < 3) return;
  dscProcessZoneStatus(2, 6);
}


// Zones 25-32 status is stored in dscOpenZones[3] and dscOpenZonesChanged[3]: Bit 0 = Zone 25 ... Bit 7 = Zone 32
void dscProcessPanel_0x3E() {
  if (!dscValidCRC()) return;
  if (DSC_ZONES < 4) return;
  dscProcessZoneStatus(3, 6);
}


/*
 *  PGM outputs 1-14 status is stored in dscPgmOutputs[]
 *
 *  dscPgmOutputs[0], Bit 0 = PGM 1 ... Bit 7 = PGM 8
 *  dscPgmOutputs[1], Bit 0 = PGM 9 ... Bit 5 = PGM 14
 */
void dscProcessPanel_0x87() {
  if (!dscValidCRC()) return;

  // Resets flag to write access code if needed when writing command output keys
  for (uint8_t partitionIndex = 0; partitionIndex < DSC_PARTITIONS; partitionIndex++) {
    dscWriteAccessCode[partitionIndex] = false;
  }

  dscPgmOutputs[0] = dscPanelData[3] & 0x03;
  dscPgmOutputs[0] |= dscPanelData[2] << 2;
  dscPgmOutputs[1] = dscPanelData[2] >> 6;
  dscPgmOutputs[1] |= (dscPanelData[3] & 0xF0) >> 2;

  for (uint8_t pgmByte = 0; pgmByte < 2; pgmByte++) {
    uint8_t pgmChanged = dscPgmOutputs[pgmByte] ^ dscPreviousPgmOutputs[pgmByte];

    if (pgmChanged != 0) {
      dscPreviousPgmOutputs[pgmByte] = dscPgmOutputs[pgmByte];
      dscPgmOutputsStatusChanged = true;
      if (!dscPauseStatus) dscStatusChanged = true;

      for (uint8_t pgmBit = 0; pgmBit < 8; pgmBit++) {
        if (bitRead(pgmChanged, pgmBit)) bitWrite(dscPgmOutputsChanged[pgmByte], pgmBit, 1);
      }
    }
  }
}


void dscProcessPanel_0xA5() {
  if (!dscValidCRC()) return;

  dscProcessTime(2);

  // Timestamp
  if (dscPanelData[6] == 0 && dscPanelData[7] == 0) {
    dscStatusChanged = true;
    dscTimestampChanged = true;
    return;
  }

  uint8_t partition = dscPanelData[3] >> 6;
  switch (dscPanelData[5] & 0x03) {
    case 0x00: dscProcessPanelStatus0(partition, 6); break;
    case 0x01: dscProcessPanelStatus1(partition, 6); break;
    case 0x02: dscProcessPanelStatus2(partition, 6); break;
  }
}


/*
 *  0xE6: Extended status, partitions 1-8
 *  Panels: PC5020, PC1616, PC1832, PC1864
 *
 *  0xE6 commands are split into multiple subcommands to handle up to 8 partitions/64 zones.
 */
void dscProcessPanel_0xE6() {
  if (!dscValidCRC()) return;

  switch (dscPanelData[2]) {
    case 0x09: dscProcessPanel_0xE6_0x09(); return;  // Zones 33-40 status
    case 0x0B: dscProcessPanel_0xE6_0x0B(); return;  // Zones 41-48 status
    case 0x0D: dscProcessPanel_0xE6_0x0D(); return;  // Zones 49-56 status
    case 0x0F: dscProcessPanel_0xE6_0x0F(); return;  // Zones 57-64 status
    case 0x1A: dscProcessPanel_0xE6_0x1A(); return;  // Panel status
  }
}


// Zones 33-40 status is stored in dscOpenZones[4] and dscOpenZonesChanged[4]: Bit 0 = Zone 33 ... Bit 7 = Zone 40
void dscProcessPanel_0xE6_0x09() {
  if (DSC_ZONES > 4) dscProcessZoneStatus(4, 3);
}


// Zones 41-48 status is stored in dscOpenZones[5] and dscOpenZonesChanged[5]: Bit 0 = Zone 41 ... Bit 7 = Zone 48
void dscProcessPanel_0xE6_0x0B() {
  if (DSC_ZONES > 5) dscProcessZoneStatus(5, 3);
}


// Zones 49-56 status is stored in dscOpenZones[6] and dscOpenZonesChanged[6]: Bit 0 = Zone 49 ... Bit 7 = Zone 56
void dscProcessPanel_0xE6_0x0D() {
  if (DSC_ZONES > 6) dscProcessZoneStatus(6, 3);
}


// Zones 57-64 status is stored in dscOpenZones[7] and dscOpenZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
void dscProcessPanel_0xE6_0x0F() {
  if (DSC_ZONES > 7) dscProcessZoneStatus(7, 3);
}


// Panel status
void dscProcessPanel_0xE6_0x1A() {

  // Panel AC power trouble
  if (dscPanelData[6] & 0x10) dscPowerTrouble = true;
  else dscPowerTrouble = false;

  if (dscPowerTrouble != dscPreviousPower) {
    dscPreviousPower = dscPowerTrouble;
    dscPowerChanged = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessPanel_0xEB() {
  if (!dscValidCRC()) return;
  if (DSC_PARTITIONS < 3) return;

  dscProcessTime(3);

  uint8_t partition;
  switch (dscPanelData[2]) {
    case 0x01: partition = 1; break;
    case 0x02: partition = 2; break;
    case 0x04: partition = 3; break;
    case 0x08: partition = 4; break;
    case 0x10: partition = 5; break;
    case 0x20: partition = 6; break;
    case 0x40: partition = 7; break;
    case 0x80: partition = 8; break;
    default: partition = 0; break;
  }

  switch (dscPanelData[7] & 0x07) {
    case 0x00: dscProcessPanelStatus0(partition, 8); break;
    case 0x01: dscProcessPanelStatus1(partition, 8); break;
    case 0x02: dscProcessPanelStatus2(partition, 8); break;
    case 0x04: dscProcessPanelStatus4(partition, 8); break;
    case 0x05: dscProcessPanelStatus5(partition, 8); break;
  }
}


void dscProcessPanelStatus0(uint8_t partition, uint8_t panelByte) {

  // Processes status messages that are not partition-specific
  if (dscPanelData[0] == 0xA5) {
    switch (dscPanelData[panelByte]) {

      // Keypad Fire alarm
      case 0x4E: {
        dscKeypadFireAlarm = true;
        if (!dscPauseStatus) dscStatusChanged = true;
        return;
      }

      // Keypad Aux alarm
      case 0x4F: {
        dscKeypadAuxAlarm = true;
        if (!dscPauseStatus) dscStatusChanged = true;
        return;
      }

      // Keypad Panic alarm
      case 0x50: {
        dscKeypadPanicAlarm = true;
        if (!dscPauseStatus) dscStatusChanged =true;
        return;
      }

      // Panel battery trouble
      case 0xE7: {
        dscBatteryTrouble = true;
        dscBatteryChanged = true;
        if (!dscPauseStatus) dscStatusChanged = true;
        return;
      }

      // Panel AC power failure
      case 0xE8: {
        dscPowerTrouble = true;
        if (dscPowerTrouble != dscPreviousPower) {
          dscPreviousPower = dscPowerTrouble;
          dscPowerChanged = true;
          if (!dscPauseStatus) dscStatusChanged = true;
        }
        return;
      }

      // Panel battery restored
      case 0xEF: {
        dscBatteryTrouble = false;
        dscBatteryChanged = true;
        if (!dscPauseStatus) dscStatusChanged = true;
        return;
      }

      // Panel AC power restored
      case 0xF0: {
        dscPowerTrouble = false;
        if (dscPowerTrouble != dscPreviousPower) {
          dscPreviousPower = dscPowerTrouble;
          dscPowerChanged = true;
          if (!dscPauseStatus) dscStatusChanged = true;
        }
        return;
      }
      default: if (partition == 0) return;
    }
  }

  // Processes partition-specific status
  if (partition > DSC_PARTITIONS) return;  // Ensures that only the configured number of partitions are processed
  uint8_t partitionIndex = partition - 1;

  // Disarmed
  if (dscPanelData[panelByte] == 0x4A ||                                       // Disarmed after alarm in memory
      dscPanelData[panelByte] == 0xE6 ||                                       // Disarmed special: keyswitch/wireless key/DLS
      (dscPanelData[panelByte] >= 0xC0 && dscPanelData[panelByte] <= 0xE4)) {  // Disarmed by access code

    dscNoEntryDelay[partitionIndex] = false;
    dscProcessArmed(partitionIndex, false);
    dscProcessAlarmStatus(partitionIndex, false);
    dscProcessEntryDelayStatus(partitionIndex, false);

    // Disarmed by access codes 1-34, 40-42
    if (dscPanelData[panelByte] >= 0xC0 && dscPanelData[panelByte] <= 0xE4) {
      uint8_t dscCode = dscPanelData[panelByte] - 0xBF;
      dscProcessPanelAccessCode(partitionIndex, dscCode, true);
    }

    return;
  }

  // Recent closing alarm
  if (dscPanelData[panelByte] == 0x4B) {
    dscProcessAlarmStatus(partitionIndex, true);
    return;
  }

  // Zone alarm, zones 1-32
  // Zone alarm status is stored using 1 bit per zone in dscAlarmZones[] and dscAlarmZonesChanged[]:
  //   dscAlarmZones[0] and dscAlarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
  //   dscAlarmZones[1] and dscAlarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
  //   ...
  //   dscAlarmZones[7] and dscAlarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
  if (dscPanelData[panelByte] >= 0x09 && dscPanelData[panelByte] <= 0x28) {
    dscProcessAlarmStatus(partitionIndex, true);
    dscProcessEntryDelayStatus(partitionIndex, false);
    dscProcessAlarmZones(panelByte, 0, 0x09, 1);
    return;
  }

  // Zone alarm restored, zones 1-32
  // Zone alarm status is stored using 1 bit per zone in dscAlarmZones[] and dscAlarmZonesChanged[]:
  //   dscAlarmZones[0] and dscAlarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
  //   dscAlarmZones[1] and dscAlarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
  //   ...
  //   dscAlarmZones[7] and dscAlarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
  if (dscPanelData[panelByte] >= 0x29 && dscPanelData[panelByte] <= 0x48) {
    dscProcessAlarmZones(panelByte, 0, 0x29, 0);
    return;
  }

  // Armed by access codes 1-34, 40-42
  if (dscPanelData[panelByte] >= 0x99 && dscPanelData[panelByte] <= 0xBD) {
    uint8_t dscCode = dscPanelData[panelByte] - 0x98;
    dscProcessPanelAccessCode(partitionIndex, dscCode, true);
    return;
  }
}


void dscProcessPanelStatus1(uint8_t partition, uint8_t panelByte) {
  if (partition == 0 || partition > DSC_PARTITIONS) return;
  uint8_t partitionIndex = partition - 1;

  switch (dscPanelData[panelByte]) {

    // Armed with no entry delay
    case 0xD2: {
      dscProcessNoEntryDelayStatus(partitionIndex, false);
      return;
    }
  }
}


void dscProcessPanelStatus2(uint8_t partition, uint8_t panelByte) {
  if (partition == 0 || partition > DSC_PARTITIONS) return;
  uint8_t partitionIndex = partition - 1;

  // Armed: stay and Armed: away
  if (dscPanelData[panelByte] == 0x9A || dscPanelData[panelByte] == 0x9B) {
    if (dscPanelData[panelByte] == 0x9A) {
      dscArmedStay[partitionIndex] = true;
      dscArmedAway[partitionIndex] = false;
    }
    else if (dscPanelData[panelByte] == 0x9B) {
      dscArmedStay[partitionIndex] = false;
      dscArmedAway[partitionIndex] = true;
    }

    dscArmed[partitionIndex] = true;
    if (dscArmed[partitionIndex] != dscPreviousArmed[partitionIndex] || dscArmedStay[partitionIndex] != dscPreviousArmedStay[partitionIndex]) {
      dscPreviousArmed[partitionIndex] = dscArmed[partitionIndex];
      dscPreviousArmedStay[partitionIndex] = dscArmedStay[partitionIndex];
      dscArmedChanged[partitionIndex] = true;
      if (!dscPauseStatus) dscStatusChanged = true;
    }

    dscProcessExitDelayStatus(partitionIndex, false);
    dscExitState[partitionIndex] = DSC_EXIT_NONE;
    dscProcessReadyStatus(partitionIndex, false);
    return;
  }

  if (dscPanelData[0] == 0xA5) {
    switch (dscPanelData[panelByte]) {

      // Activate stay/away zones
      case 0x99: {
        dscArmed[partitionIndex] = true;
        dscArmedAway[partitionIndex] = true;
        dscArmedStay[partitionIndex] = false;
        dscArmedChanged[partitionIndex] = true;
        if (!dscPauseStatus) dscStatusChanged = true;
        return;
      }

      // Armed with no entry delay
      case 0x9C: {
        dscProcessNoEntryDelayStatus(partitionIndex, true);
        dscProcessReadyStatus(partitionIndex, false);
        return;
      }
    }
  }
}


void dscProcessPanelStatus4(uint8_t partition, uint8_t panelByte) {
  if (partition == 0 || partition > DSC_PARTITIONS) return;
  uint8_t partitionIndex = partition - 1;

  // Zone alarm, zones 33-64
  // Zone alarm status is stored using 1 bit per zone in dscAlarmZones[] and dscAlarmZonesChanged[]:
  //   dscAlarmZones[0] and dscAlarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
  //   dscAlarmZones[1] and dscAlarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
  //   ...
  //   dscAlarmZones[7] and dscAlarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
  if (dscPanelData[panelByte] <= 0x1F) {
    dscProcessAlarmStatus(partitionIndex, true);
    dscProcessEntryDelayStatus(partitionIndex, false);
    dscProcessAlarmZones(panelByte, 4, 0, 1);
    return;
  }

  // Zone alarm restored, zones 33-64
  // Zone alarm status is stored using 1 bit per zone in dscAlarmZones[] and dscAlarmZonesChanged[]:
  //   dscAlarmZones[0] and dscAlarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
  //   dscAlarmZones[1] and dscAlarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
  //   ...
  //   dscAlarmZones[7] and dscAlarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
  if (dscPanelData[panelByte] >= 0x20 && dscPanelData[panelByte] <= 0x3F) {
    dscProcessAlarmZones(panelByte, 4, 0x20, 0);
    return;
  }
}


void dscProcessPanelStatus5(uint8_t partition, uint8_t panelByte) {
  if (partition == 0 || partition > DSC_PARTITIONS) return;
  uint8_t partitionIndex = partition - 1;

  /*
   *  Armed by access codes 35-95
   *  0x00 - 0x04: Access codes 35-39
   *  0x05 - 0x39: Access codes 43-95
   */
  if (dscPanelData[panelByte] <= 0x39) {
    uint8_t dscCode = dscPanelData[panelByte] + 0x23;
    dscProcessPanelAccessCode(partitionIndex, dscCode, false);
    return;
  }

  /*
   *  Disarmed by access codes 35-95
   *  0x3A - 0x3E: Access codes 35-39
   *  0x3F - 0x73: Access codes 43-95
   */
  if (dscPanelData[panelByte] >= 0x3A && dscPanelData[panelByte] <= 0x73) {
    uint8_t dscCode = dscPanelData[panelByte] - 0x17;
    dscProcessPanelAccessCode(partitionIndex, dscCode, false);
    return;
  }
}


void dscProcessReadyStatus(uint8_t partitionIndex, bool status) {
  dscReady[partitionIndex] = status;
  if (dscReady[partitionIndex] != dscPreviousReady[partitionIndex]) {
    dscPreviousReady[partitionIndex] = dscReady[partitionIndex];
    dscReadyChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessAlarmStatus(uint8_t partitionIndex, bool status) {
  dscAlarm[partitionIndex] = status;
  if (dscAlarm[partitionIndex] != dscPreviousAlarm[partitionIndex]) {
    dscPreviousAlarm[partitionIndex] = dscAlarm[partitionIndex];
    dscAlarmChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessExitDelayStatus(uint8_t partitionIndex, bool status) {
  dscExitDelay[partitionIndex] = status;
  if (dscExitDelay[partitionIndex] != dscPreviousExitDelay[partitionIndex]) {
    dscPreviousExitDelay[partitionIndex] = dscExitDelay[partitionIndex];
    dscExitDelayChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessEntryDelayStatus(uint8_t partitionIndex, bool status) {
  dscEntryDelay[partitionIndex] = status;
  if (dscEntryDelay[partitionIndex] != dscPreviousEntryDelay[partitionIndex]) {
    dscPreviousEntryDelay[partitionIndex] = dscEntryDelay[partitionIndex];
    dscEntryDelayChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessNoEntryDelayStatus(uint8_t partitionIndex, bool status) {
  dscNoEntryDelay[partitionIndex] = status;
  if (dscNoEntryDelay[partitionIndex] != dscPreviousNoEntryDelay[partitionIndex]) {
    dscPreviousNoEntryDelay[partitionIndex] = dscNoEntryDelay[partitionIndex];
    dscArmedChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessZoneStatus(uint8_t zonesByte, uint8_t panelByte) {
  dscOpenZones[zonesByte] = dscPanelData[panelByte];
  uint8_t zonesChanged = dscOpenZones[zonesByte] ^ dscPreviousOpenZones[zonesByte];
  if (zonesChanged != 0) {
    dscPreviousOpenZones[zonesByte] = dscOpenZones[zonesByte];
    dscOpenZonesStatusChanged = true;
    if (!dscPauseStatus) dscStatusChanged = true;

    for (uint8_t zoneBit = 0; zoneBit < 8; zoneBit++) {
      if (bitRead(zonesChanged, zoneBit)) {
        bitWrite(dscOpenZonesChanged[zonesByte], zoneBit, 1);
        if (bitRead(dscPanelData[panelByte], zoneBit)) bitWrite(dscOpenZones[zonesByte], zoneBit, 1);
        else bitWrite(dscOpenZones[zonesByte], zoneBit, 0);
      }
    }
  }
}

void dscProcessTime(uint8_t panelByte) {
  uint8_t year3 = dscPanelData[panelByte] >> 4;
  uint8_t year4 = dscPanelData[panelByte] & 0x0F;
  dscYear = (year3 * 10) + year4;
  if (year3 >= 7) dscYear += 1900;
  else dscYear += 2000;
  dscMonth = dscPanelData[panelByte + 1] << 2; dscMonth >>=4;
  uint8_t day1 = dscPanelData[panelByte + 1] << 6; day1 >>= 3;
  uint8_t day2 = dscPanelData[panelByte + 2] >> 5;
  dscDay = day1 | day2;
  dscHour = dscPanelData[panelByte + 2] & 0x1F;
  dscMinute = dscPanelData[panelByte + 3] >> 2;
}


void dscProcessAlarmZones(uint8_t panelByte, uint8_t startByte, uint8_t zoneCountOffset, uint8_t writeValue) {
  uint8_t maxZones = DSC_ZONES * 8;
  if (maxZones > 32) {
    if (startByte < 4) maxZones = 32;
    else maxZones -= 32;
  }
  else if (startByte >= 4) return;

  for (uint8_t zoneCount = 0; zoneCount < maxZones; zoneCount++) {
    if (dscPanelData[panelByte] == zoneCount + zoneCountOffset) {
      for (uint8_t zoneRange = 0; zoneRange <= 3; zoneRange++) {
        if (zoneCount >= (zoneRange * 8) && zoneCount < ((zoneRange * 8) + 8)) {
          dscProcessAlarmZonesStatus(startByte + zoneRange, zoneCount, writeValue);
        }
      }
    }
  }
}


void dscProcessAlarmZonesStatus(uint8_t zonesByte, uint8_t zoneCount, uint8_t writeValue) {
  uint8_t zonesRange = zonesByte;
  if (zonesRange >= 4) zonesRange -= 4;

  bitWrite(dscAlarmZones[zonesByte], (zoneCount - (zonesRange * 8)), writeValue);

  if (bitRead(dscPreviousAlarmZones[zonesByte], (zoneCount - (zonesRange * 8))) != writeValue) {
    bitWrite(dscPreviousAlarmZones[zonesByte], (zoneCount - (zonesRange * 8)), writeValue);
    bitWrite(dscAlarmZonesChanged[zonesByte], (zoneCount - (zonesRange * 8)), 1);

    dscAlarmZonesStatusChanged = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessArmed(uint8_t partitionIndex, bool armedStatus) {
  dscArmedStay[partitionIndex] = armedStatus;
  dscArmedAway[partitionIndex] = armedStatus;
  dscArmed[partitionIndex] = armedStatus;

  if (dscArmed[partitionIndex] != dscPreviousArmed[partitionIndex]) {
    dscPreviousArmed[partitionIndex] = dscArmed[partitionIndex];
    dscArmedChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}


void dscProcessPanelAccessCode(uint8_t partitionIndex, uint8_t dscCode, bool accessCodeIncrease) {
  if (accessCodeIncrease) {
    if (dscCode >= 35) dscCode += 5;
  }
  else {
    if (dscCode >= 40) dscCode += 3;
  }

  dscAccessCode[partitionIndex] = dscCode;
  if (dscAccessCode[partitionIndex] != dscPreviousAccessCode[partitionIndex]) {
    dscPreviousAccessCode[partitionIndex] = dscAccessCode[partitionIndex];
    dscAccessCodeChanged[partitionIndex] = true;
    if (!dscPauseStatus) dscStatusChanged = true;
  }
}
