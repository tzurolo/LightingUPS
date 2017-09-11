//
// System Mode
//

#include "SystemMode.h"

#include "EEPROMStorage.h"
#include <avr/io.h>

#define MODE_SWITCH_DDR    DDRB
#define MODE_SWITCH_PORT   PORTB
#define MODE_SWITCH_INPORT PINB
#define MODE_SWITCH_PIN    PB1


void SystemMode_Initialize (void)
{
    // set mode switch pin to be an input and enable the pullup
    MODE_SWITCH_DDR &= ~(1 << MODE_SWITCH_PIN);
    MODE_SWITCH_PORT |= (1 << MODE_SWITCH_PIN);
}

SystemMode_mode SystemMode_currentMode (void)
{
    SystemMode_mode mode = SystemMode_modeSetting();
    if (mode == m_switch) {
        // read the switch
        const bool switchState = 
            ((MODE_SWITCH_INPORT & (1 << MODE_SWITCH_PIN)) != 0);
        mode = (switchState)
            ? m_backup
            : m_primary;
    }

    return mode;
}

SystemMode_mode SystemMode_modeSetting (void)
{
    return EEPROMStorage_mode;
}

void SystemMode_setModeSetting (
    const SystemMode_mode newSetting)
{
    EEPROMStorage_setMode(newSetting);
}


