//
// Integer Limit Functions
// 
// Utility functions that clamp integer values to limits
//
//
#ifndef INTLIMIT_LOADED
#define INTLIMIT_LOADED

#include "inttypes.h"

// clamps the given value to be between min_value and max_value
extern int8_t int8_limit (
   const int value,
   const int min_value,
   const int max_value);

// clamps the given value to be between min_value and max_value
extern int16_t int16_limit (
   const int value,
   const int min_value,
   const int max_value);

// clamps the given value to be between min_value and max_value
extern int32_t int32_limit (
   const int32_t value,
   const int32_t min_value,
   const int32_t max_value);

// clamps the given value to be between min_value and max_value,
// which are expected to be positive numbers, or between -max_value
// and -min_value
extern int32_t int32_limit_magnitude (
   const int32_t value,
   const int32_t min_abs_value,
   const int32_t max_abs_value);


#endif   /* INTLIMIT_LOADED */
