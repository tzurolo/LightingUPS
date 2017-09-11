//
//  Calibration Manager
//
//  Manages workflow for setting voltage parameters in EEPROM
//
#ifndef CALIBRATIONMANAGER_H
#define CALIBRATIONMANAGER_H

typedef enum {
    cm_waitingForButtonPress,
    cm_buttonDebounce,
    cm_acknowldegeSet,
    cm_buttonRelease
} CalibrationManager_state;

extern void CalibrationManager_Initialize (void);

extern void CalibrationManager_task (void);

#endif      // CALIBRATIONMANAGER_H