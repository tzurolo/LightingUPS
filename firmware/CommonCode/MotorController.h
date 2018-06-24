//
// Motor Controller
//
// Provides a form of closed-loop angle positioning control for up to two
// gearmmotor shafts using an optical encoder on the motor shaft. Motor 1
// uses optical encoder 1 and motor 2 uses optical encoder 2
//
#ifndef MOTORCONTROLLER_LOADED
#define MOTORCONTROLLER_LOADED

#include "inttypes.h"
#include "booltype.h"

// to be called once upon power on reset
void MotorController_init ();

#define MotorController_max_fwd_speed 255
#define MotorController_max_rev_speed -254

// to be called frequently for each motor, such as from the program's mainloop.
// manages and executes the commands initiated by the functions below.
extern void MotorController_update (
   const uint8_t which_motor,
   Bool *motor_has_just_stopped);

extern void MotorController_get_current_angle (
   const uint8_t which_motor,
   int32_t *current_angle,
   uint8_t *num_errors);

// the speed set here is the signed speed the motor will run when _run is
// called. Postive values increase angle, negative values decrease angle,
// zero coasts. The speed set here is also the speed at which the motor runs
// when _move_to_angle is called. In this case the sign of the speed is
// ignored and the controller uses whatever sign is necessary to move toward
// the target angle
extern void MotorController_set_speed (
   const uint8_t which_motor,
   const int16_t new_speed);

// Starts runnung the motor at the speed set by the last call to _set_speed()
// until it reaches the given angle. Subsequent calls to _update() will tell
// when the motor has reached (stopped at) the new angle 
extern void MotorController_move_to_angle (
   const uint8_t which_motor,
   const int32_t new_angle);

// Starts running the motor indefinitely at the speed set by the last call
// to _set_speed(). call _brake_to_stop() to stop running
extern void MotorController_run (
   const uint8_t which_motor);

// Starts braking the motor. Subsequent calls to _update() will tell when
// the motor has come to a complete stop
extern void MotorController_brake_to_stop (
   const uint8_t which_motor);

// Starts searching for the optical switch index mark, and resets the motor's
// angle reading to zero when it finds the mark. The process is complete
// when _update() indicates that the motor has stopped.
extern void MotorController_find_index_mark (
   const uint8_t which_motor);

#endif   // MOTORCONTROLLER_LOADED
