//
// Motor Controller Utilities
//
// Commonly used functions used in conjunction with motor control
//
//
#ifndef MOTORCONTROLLERUTILS_LOADED
#define MOTORCONTROLLERUTILS_LOADED

#include "inttypes.h"
  
extern void report_motor_angle (
   const char reply_char,
   const int32_t motor_angle,
   const uint8_t num_errors);

extern void query_motor_position (
   const uint8_t motor_number,
   const char reply_char);

#endif   /* MOTORCONTROLLERUTILS_LOADED */
