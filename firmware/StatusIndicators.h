//  Status Indicators
//
//  Controls two outputs that indicate the internal status of the UPS
//  These outputs are connected to two LEDs and they also go to the
//  box being powered for remote status reporting.
//

#ifndef STATUSINDICATORS_H
#define STATUSINDICATORS_H

extern void StatusIndicators_Initialize (void);

extern void StatusIndicators_task (void);

extern void StatusIndicators_sendStatusMesssage (void);

#endif      // STATUSINDICATORS_H
