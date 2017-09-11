//
// Optical Encoder
//

#include "OpticalEncoder.h"
#include "OpticalEncoderConfig.h"

#include "booltype.h"
#include "device.h"
#include "UART_polled.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// defining this to be a 1 will cause pin PC5 to be set at the start of the pin change
// interrupt handler and cleared at the end. Connecting PC5 to a scope allows
// measurement of how much time the interrupt handler takes
#define MEASURE_INTERRUPT_TIME 0
// defining this to be 1 will cause the 
#define DISPLAY_ENCODER_AND_SWITCH_BITS 0

// encoder 1 bits are in Port B
#define OPTICAL_ENCODER_1_BITS ((port_b >> OPTICAL_ENCODER_1_LSB) & 0x03)
#if OPTICAL_SWITCHES_1_ENABLED
#define OPTICAL_SWITCHES_1_ZERO_WHEN_ON ((~port_b) & OPTICAL_SWITCHES_1_MASK) 
#else
#define OPTICAL_SWITCHES_1_ZERO_WHEN_ON 1
#endif

// encoder 2 bits are in Port C
#define OPTICAL_ENCODER_2_BITS ((port_c >> OPTICAL_ENCODER_2_LSB) & 0x03)
#if OPTICAL_SWITCHES_1_ENABLED
#define OPTICAL_SWITCHES_2_ZERO_WHEN_ON ((~port_c) & OPTICAL_SWITCHES_1_MASK) 
#else
#define OPTICAL_SWITCHES_2_ZERO_WHEN_ON 1
#endif

#define MAX_TIME_AT_ANGLE 0xFFFF

typedef struct {
   uint16_t timer_start_value;
   uint8_t timer_rollover_counter; // number of times the 16-bit timer rolled over
   uint16_t time_at_angle;
   int8_t last_increment;
   uint8_t last_encoder_bits;
   int32_t current_angle;
   uint16_t num_errors;
   uint8_t encoder_bits_to_qualify_switches;
   Bool reset_at_index_mark;
   Bool index_mark_detected;
   } OpticalEncoder;

static volatile OpticalEncoder encoder1;
static volatile OpticalEncoder encoder2;

static void init_OpticalEncoder (
   const uint8_t initial_encoder_bits,
   const uint8_t encoder_bits_for_switches,
   volatile OpticalEncoder *encoder)
   {
   encoder->timer_start_value = 0;
   encoder->timer_rollover_counter = 2;
   encoder->time_at_angle = MAX_TIME_AT_ANGLE;
   encoder->last_increment = 0;
   encoder->last_encoder_bits = initial_encoder_bits;
   encoder->current_angle = 0;
   encoder->num_errors = 0;
   encoder->encoder_bits_to_qualify_switches = encoder_bits_for_switches;
   encoder->reset_at_index_mark = false;
   encoder->index_mark_detected = false;
   }

/*

 encoder analysis bit assignments
    +--------+--------+--------+--------+--------+--------+--------+--------+
 bit|   7    |   6    |   5    |   4    |   3    |   2    |   1    |   0    |
    +--------+--------+--------+--------+--------+--------+--------+--------+
    |        |        |        |        |current |current | last   | last   |
    |        |        |        |        | ch B   | ch A   | ch B   | ch A   |
    +--------+--------+--------+--------+--------+--------+--------+--------+
*/


// valid transitions are those that follow the "gray" sequence (0, 1, 3, 2, 0, 1,...)
static const int8_t encoder_increment_lookup_table[16] = {
    0,   // 0000  0 -> 0
   -1,   // 0001  1 -> 0
    1,   // 0010  2 -> 0
    2,   // 0011  3 -> 0, but we don't know in what direction
    1,   // 0100  0 -> 1
    0,   // 0101  1 -> 1
    2,   // 0110  2 -> 1, but we don't know in what direction
   -1,   // 0111  3 -> 1
   -1,   // 1000  0 -> 2
    2,   // 1001  1 -> 2, but we don't know in what direction
    0,   // 1010  2 -> 2
    1,   // 1011  3 -> 2
    2,   // 1100  0 -> 3, but we don't know in what direction
    1,   // 1101  1 -> 3
   -1,   // 1110  2 -> 3
    0    // 1111  3 -> 3
   };

static inline void compute_time_at_angle (
   const uint16_t current_timer_value,
   const uint16_t timer_start_value,
   const uint8_t timer_rollover_counter,
   volatile uint16_t *time_at_angle)
   {
   if (timer_rollover_counter == 0)
      // the timer did not roll over since last capture
      *time_at_angle = current_timer_value - timer_start_value;
   else if (timer_rollover_counter == 1)
      {  // the timer rolled over once
      if (timer_start_value > current_timer_value)
         *time_at_angle = current_timer_value - timer_start_value;
      else
         *time_at_angle = MAX_TIME_AT_ANGLE;
      }
   else  // timer rolled over more than once
      *time_at_angle = MAX_TIME_AT_ANGLE;
   }

static inline void capture_and_reset_timer (
   volatile OpticalEncoder *the_encoder)
   {
   // capture the current timer
   const uint16_t current_timer_value = TCNT1;

   compute_time_at_angle(current_timer_value, the_encoder->timer_start_value,
      the_encoder->timer_rollover_counter, &the_encoder->time_at_angle);

   // reset timer state variables
   the_encoder->timer_start_value = current_timer_value;
   the_encoder->timer_rollover_counter = 0;
   }

void OpticalEncoder_init ()
   {
   uint8_t port_b = PINB;
   uint8_t port_c = PINC;
   // initialize encoder objects
   init_OpticalEncoder(OPTICAL_ENCODER_1_BITS, OPTICAL_SWITCHES_1_ENCODER_VALUE,
      &encoder1);
   init_OpticalEncoder(OPTICAL_ENCODER_2_BITS, OPTICAL_SWITCHES_2_ENCODER_VALUE,
      &encoder2);

   // enable pin change interrupts
   if (OPTICAL_ENCODER_1_ENABLED)
      {
      PCICR |= (1 << PCIE0);   // enable port B pin change interrupts
      PCMSK0 |= (0x03 << OPTICAL_ENCODER_1_LSB);   // enable pin changes on
                                                   // encoder bits
      }
   if (OPTICAL_ENCODER_2_ENABLED)
      {
      PCICR |= (1 << PCIE1);   // enable port C pin change interrupts
      PCMSK1 |= (0x03 << OPTICAL_ENCODER_2_LSB);   // enable pin changes on
                                                   // encoder bits
      }

   // enable Timer/Counter 1 (16-bit) to be clocked internally
   TCCR1B = (TCCR1B & 0xF8) | 4; // prescale by 256
   TCNT1 = 0;  // start the timer at 0
   TIFR1 |= (1 << TOV1);  // clear the overflow flag
   // enable timer overflow interrupt
   TIMSK1 |= (1 << TOIE1);

#if MEASURE_INTERRUPT_TIME
   // for timing measurement, set bit PC5 to be an output.
   DDRC |= (1 << PC5);
#endif
   }

static inline void OpticalEncoder_update (
   const uint8_t new_encoder_bits,
   const uint8_t new_optical_switch_zero_when_on,
   volatile OpticalEncoder *encoder)
   {
   uint8_t encoder_lookup_index;
   int8_t new_encoder_increment;

   // decode the bit pattern from the encoder, and compare to last reading
   encoder_lookup_index =
      (new_encoder_bits << 2) |
      (encoder->last_encoder_bits & 0x03);
   new_encoder_increment = encoder_increment_lookup_table[encoder_lookup_index];

   if (new_encoder_increment == 2)
      {  // we skipped a transition, and so can't tell whether to increment or
         // decrement
      // we assume we're still going in the same direction as the last increment
      if (encoder->last_increment == -1)
         new_encoder_increment = -2;
      else if (encoder->last_increment != 1)
         ++encoder->num_errors;
      }
   encoder->current_angle += new_encoder_increment;

   // detect when we are at a new position and capture time since last position
   if ((new_encoder_bits == 0) &&      // ensure consistent timing by always measuring at only one encoder state
       (new_encoder_increment != 0))   // we're at a new position
      capture_and_reset_timer(encoder);

   // detect the index mark, if enabled
   if ((encoder->reset_at_index_mark) &&
       (new_encoder_bits == encoder->encoder_bits_to_qualify_switches) &&
       (new_optical_switch_zero_when_on == 0))
      {
      // indicate that we have detected the mark and reset the angle
      encoder->index_mark_detected = true;
      encoder->current_angle = 0;
      // once we detect the index mark automatically disable detection
      encoder->reset_at_index_mark = false;
      }

   // update state for next iteration
   encoder->last_increment = new_encoder_increment;
   encoder->last_encoder_bits = new_encoder_bits;

#if DISPLAY_ENCODER_AND_SWITCH_BITS
   {
   char buf[32];
   UART_write_string("e: ");
   itoa(new_encoder_bits, buf, 2);
   UART_write_string(buf);
   UART_write_string(", s: ");
   itoa(new_optical_switch_zero_when_on, buf, 2);
   UART_write_string(buf);
   UART_write_byte(0x0D);		// cr
   UART_write_byte(0x0A);		// lf
   }
#endif
   }

void OpticalEncoder_enable_reset_at_index_mark (
   const uint8_t which_encoder)
   {
   // select the encoder based on the caller's selection
   volatile OpticalEncoder* the_encoder = (which_encoder == 1) ? &encoder1 : &encoder2;
   the_encoder->reset_at_index_mark = true;
   the_encoder->index_mark_detected = false;
   }

// disables the reset of the current angle when the index mark is detected
void OpticalEncoder_disable_reset_at_index_mark (
   const uint8_t which_encoder)
   {
   // select the encoder based on the caller's selection
   volatile OpticalEncoder* the_encoder = (which_encoder == 1) ? &encoder1 : &encoder2;
   the_encoder->reset_at_index_mark = false;
   }

Bool OpticalEncoder_index_mark_detected (
   const uint8_t which_encoder)
   {
   // select the encoder based on the caller's selection
   volatile OpticalEncoder* the_encoder = (which_encoder == 1) ? &encoder1 : &encoder2;
   return the_encoder->index_mark_detected;
   }

void OpticalEncoder_get_angle_and_time (
   const uint8_t which_encoder,   // 1 or 2
   int32_t *angle,
   uint16_t *time_at_angle,
   uint8_t *num_errors)
   {
   uint16_t current_timer_value;
   uint16_t current_timer_start_value;
   uint8_t current_rollover_counter;
   uint16_t current_time_at_angle;
   uint16_t new_time_at_angle;

   // select the encoder based on the caller's selection
   volatile OpticalEncoder* the_encoder = (which_encoder == 1) ? &encoder1 : &encoder2;

   // get the current state of the encoder
   cli();
   current_timer_value = TCNT1;
   current_timer_start_value = the_encoder->timer_start_value;
   current_rollover_counter = the_encoder->timer_rollover_counter;
   current_time_at_angle = the_encoder->time_at_angle;
   *angle = the_encoder->current_angle;
   *num_errors = the_encoder->num_errors;
   the_encoder->num_errors = 0;
   sei();

   // compute the new time at angle
   compute_time_at_angle(current_timer_value, current_timer_start_value,
      current_rollover_counter, &new_time_at_angle); 
   // return the longer of the new time at angle and the current time at angle
   if (new_time_at_angle > current_time_at_angle)
      current_time_at_angle = new_time_at_angle;
   *time_at_angle = current_time_at_angle;
   }


void OpticalEncoder_get_angle (
   const uint8_t which_encoder,   // 1 or 2
   volatile int32_t *angle)
   {
   // select the encoder based on the caller's selection
   volatile OpticalEncoder* the_encoder = (which_encoder == 1) ? &encoder1 : &encoder2;
   cli();
   *angle = the_encoder->current_angle;
   sei();
   }

// handler for encoder bit change in port B (encoder 1)
ISR(SIG_PIN_CHANGE0)
   {
   uint8_t port_b;
#if MEASURE_INTERRUPT_TIME
   PORTC |= (1 << PC5);
#endif
   port_b = PINB;
   OpticalEncoder_update(OPTICAL_ENCODER_1_BITS,
      OPTICAL_SWITCHES_1_ZERO_WHEN_ON, &encoder1);
#if MEASURE_INTERRUPT_TIME
   PORTC &= ~(1 << PC5);
#endif
   }

// handler for encoder bit change in port C (encoder 2)
ISR(SIG_PIN_CHANGE1)
   {
   uint8_t port_c;
   port_c = PINC;
   OpticalEncoder_update(OPTICAL_ENCODER_2_BITS,
      OPTICAL_SWITCHES_2_ZERO_WHEN_ON, &encoder2);
   }

ISR(SIG_OVERFLOW1)
   {
   // increment rollover counters
#if OPTICAL_ENCODER_1_ENABLED
   if (encoder1.timer_rollover_counter < 2)
      ++encoder1.timer_rollover_counter;
#endif

#if OPTICAL_ENCODER_2_ENABLED
   if (encoder2.timer_rollover_counter < 2)
      ++encoder2.timer_rollover_counter;
#endif
   }
