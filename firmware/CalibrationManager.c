//
//  Calibration Manager
//
//  Manages workflow for setting voltage parameters in EEPROM
//
//  uses pins
//      PA5 - cal button
//      PA7 - voltage selector bit 0
//      PB2 - voltage selector bit 1
//

#include "CalibrationManager.h"

static CalibrationManager_state cmState = cm_waitingForButtonPress;

void CalibrationManager_Initialize (void)
{
    // set value selector and button pins as inputs with pullup
}

void CalibrationManager_task (void)
{
}

