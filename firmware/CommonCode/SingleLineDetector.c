
#include "SingleLineDetector.h"

#include "SingleLineDetectorConfig.h"
#include "OpticalEncoder.h"
#include "device.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

typedef struct {
   Bool enabled;
   Bool transition_detected;
   Bool current_state;
   int32_t angle_at_transition;
   } SingleLineDetector;

static volatile SingleLineDetector detector1;

static void init_detector (
   volatile SingleLineDetector* detector)
   {
   detector->enabled = false;
   detector->transition_detected = false;
   detector->current_state = false;
   detector->angle_at_transition = 0;
   }

void SingleLineDetector_init ()
   {
   init_detector(&detector1);
   }

void SingleLineDetector_enable ()
   {
   detector1.enabled = true;

   // enable pin change interrupts
   PCICR |= (1 << PCIE2);   // enable port D pin change interrupts
   PCMSK2 |= (1 << SINGLE_LINE_DETECTOR_1_BIT); // enable pin changes on
                                                // detector bit
   }

void SingleLineDetector_disable ()
   {
   detector1.enabled = false;

   // disable pin change interupts
   PCICR &= ~(1 << PCIE2);   // disable port D pin change interrupts
   PCMSK2 &= ~(1 << SINGLE_LINE_DETECTOR_1_BIT); // disable pin changes on
                                                 // detector bit
   }

extern Bool SingleLineDetector_transition_detected (
    Bool *new_state,
    int32_t *angle)
    {
    Bool transition_occurred = false;

    if (detector1.enabled && detector1.transition_detected)
      {
      transition_occurred = true;
      *new_state = detector1.current_state;
      *angle = detector1.angle_at_transition;
      detector1.transition_detected = false;
      }

    return transition_occurred;
    }

// handler for line detector bit change in port D
ISR(SIG_PIN_CHANGE2)
   {
   uint8_t port_d;
   port_d = PIND;
   detector1.transition_detected = true;
   detector1.current_state =
      (port_d >> SINGLE_LINE_DETECTOR_1_BIT) & 1;
   OpticalEncoder_get_angle(
      SINGLE_LINE_DETECTOR_1_ENCODER,
      &detector1.angle_at_transition);
   }

