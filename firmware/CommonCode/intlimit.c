
#include "intlimit.h"

int8_t int8_limit (
   const int value,
   const int min_value,
   const int max_value)
   {
   return (value < min_value)
          ? min_value
          : (value > max_value)
            ? max_value
            : value;
   }

int16_t int16_limit (
   const int value,
   const int min_value,
   const int max_value)
   {
   return (value < min_value)
          ? min_value
          : (value > max_value)
            ? max_value
            : value;
   }

int32_t int32_limit (
   const int32_t value,
   const int32_t min_value,
   const int32_t max_value)
   {
   return (value < min_value)
          ? min_value
          : (value > max_value)
            ? max_value
            : value;
   }

int32_t int32_limit_magnitude (
   const int32_t value,
   const int32_t min_abs_value,
   const int32_t max_abs_value)
   {
   int32_t limited_value = value;

   if (limited_value >= 0)
      {
      if (limited_value < min_abs_value)
         limited_value = min_abs_value;
      else if (limited_value > max_abs_value)
         limited_value = max_abs_value;
      }
   else
      {
      if (limited_value > -min_abs_value)
         limited_value = -min_abs_value;
      else if (limited_value < -max_abs_value)
         limited_value = -max_abs_value;
      }

   return limited_value;
   }
