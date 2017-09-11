//
// Single Line Detector
//
//   Provides an interface to the SLD-01 Reflective IR sensor.
//   Pin-change interrupts are triggered when the line detector changes
//   state.
//   Which pin the detector is connected to and other information is
//   specified in SingleLineDetectorConfig.h
//
#ifndef SINGLELINEDETECTOR_LOADED
#define SINGLELINEDETECTOR_LOADED

#include "booltype.h"
#include <inttypes.h>

// 
extern void SingleLineDetector_init ();

extern void SingleLineDetector_enable ();

extern void SingleLineDetector_disable ();

// returns true when a transition from light to dark or dark to
// light has been detected. The optical encoder angle at the
// instant the transition occurred is returned.
extern Bool SingleLineDetector_transition_detected (
    Bool *new_state,    // true = light, false = dark
    int32_t *angle);

#endif   // SINGLELINEDETECTOR_LOADED
