//
// System Mode
//
//  Represents the operational mode of the Lighting UPS system
//
//  Primary (P) - the system is the primary (or only) lighting system of
//                the room
//  Backup (B)  - the system provides lighting in the room only when the
//                AC mains power is down
//  Switch (S)  - the mode is determined by the state of the mode switch
//                on the controller box
//
#ifndef SYSTEMMODE_H
#define SYSTEMMODE_H

#include <stdint.h>
#include <stdbool.h>

typedef enum mode_enum {
    m_primary = 'P',
    m_backup = 'B',
    m_switch = 'S'
} SystemMode_mode;

extern void SystemMode_Initialize (void);

// returns the current mode, which is determined from the
// system mode setting, and if the setting is 'Switch' reads
// the switch and determines the mode from that
extern SystemMode_mode SystemMode_currentMode (void);

extern SystemMode_mode SystemMode_modeSetting (void);

extern void SystemMode_setModeSetting (
    const SystemMode_mode newSetting);

#endif      /* SYSTEMMODE_H */
