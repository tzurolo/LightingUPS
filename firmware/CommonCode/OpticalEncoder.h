//
// Optical Encoder
//
// This "class" represents an optical shaft encoder that generates quadrature
// signals to indicate changes in shaft angle.
//
// Currently supports up to two encoders, one on port B and one on port C. The
// specification of which encoder is being used (1, 2, or both) is controlled by
// the definitions in OpticalEncoderConfig.h
//
#ifndef OPTICALENCODER_LOADED
#define OPTICALENCODER_LOADED

#include "booltype.h"
#include <inttypes.h>

// index mark reset is disabled
extern void OpticalEncoder_init ();

// after this function is called the current angle will be reset to zero
// when the index mark (detected by the optical switches) is encountered.
// This mode is disabled automatically when it detects the index mark.
extern void OpticalEncoder_enable_reset_at_index_mark (
   const uint8_t which_encoder);  // 1 or 2

// disables the reset of the current angle when the index mark is detected
extern void OpticalEncoder_disable_reset_at_index_mark (
   const uint8_t which_encoder);  // 1 or 2

extern Bool OpticalEncoder_index_mark_detected (
   const uint8_t which_encoder);  // 1 or 2

// returns current angle, time at angle, number of errors, and clears error count
extern void OpticalEncoder_get_angle_and_time (
   const uint8_t which_encoder,   // 1 or 2
   int32_t *angle,
   uint16_t *time_at_angle,
   uint8_t *num_errors);

// returns the raw encoder angle disregarding error detection
extern void OpticalEncoder_get_angle (
   const uint8_t which_encoder,   // 1 or 2
   volatile int32_t *angle);

#endif   // OPTICALENCODER_LOADED
