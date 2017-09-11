
#include "MotorControllerUtils.h"

#include "MotorController.h"
#include "UART_polled.h"
#include <stdlib.h>

void report_motor_angle (
   const char reply_char,
   const int32_t motor_angle,
   const uint8_t num_errors)
   {
   UART_write_byte(reply_char);
   if (num_errors > 0)
      UART_write_byte('E');
   char angle_buffer[16];
   ltoa(motor_angle, angle_buffer, 10);
   UART_write_string(angle_buffer);
	UART_write_byte(0x0A);		// lf
   }

void query_motor_position (
   const uint8_t motor_number,
   const char reply_char)
   {
   int32_t motor_angle;
   uint8_t num_errors;
   MotorController_get_current_angle(motor_number, &motor_angle, &num_errors);
   report_motor_angle(reply_char, motor_angle, num_errors);
   }

