#include "dscKeybus.h"


// Example task that processes and prints the security system status to a serial
// interface. This demonstrates how to determine if the security system status
// has changed, what has changed, and how to take action based on those changes.
void dscExample() {
  while(1) {

    // Blocks this task until valid panel data is available
    xSemaphoreTake(dscDataAvailable, portMAX_DELAY);

    // If the Keybus data buffer is exceeded, the program is too busy to process all Keybus commands.  Call
    // dscProcess() more often, or increase DSC_BUFFER_SIZE in the library: dscKeybus.h
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
      for (uint8_t partition = 0; partition < DSC_PARTITIONS; partition++) {

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
      //   dscOpenZones[0] and dscOpenZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
      //   dscOpenZones[1] and dscOpenZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
      //   ...
      //   dscOpenZones[7] and dscOpenZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
      if (dscOpenZonesStatusChanged) {
        dscOpenZonesStatusChanged = false;                           // Resets the open zones status flag
        for (uint8_t zoneGroup = 0; zoneGroup < DSC_ZONES; zoneGroup++) {
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
      // Zone alarm status is stored in the dscAlarmZones[] and dscAlarmZonesChanged[] arrays using 1 bit per zone, up to 64 zones
      //   dscAlarmZones[0] and dscAlarmZonesChanged[0]: Bit 0 = Zone 1 ... Bit 7 = Zone 8
      //   dscAlarmZones[1] and dscAlarmZonesChanged[1]: Bit 0 = Zone 9 ... Bit 7 = Zone 16
      //   ...
      //   dscAlarmZones[7] and dscAlarmZonesChanged[7]: Bit 0 = Zone 57 ... Bit 7 = Zone 64
      if (dscAlarmZonesStatusChanged) {
        dscAlarmZonesStatusChanged = false;                           // Resets the alarm zones status flag
        for (uint8_t zoneGroup = 0; zoneGroup < DSC_ZONES; zoneGroup++) {
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
