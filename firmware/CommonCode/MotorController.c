//
// Motor Controller
//

#include "MotorController.h"

#include "device.h"
#include "pwm.h"
#include "OpticalEncoder.h"
#include <stdlib.h>
#include <avr/pgmspace.h>

// the number of timer ticks that must elapse without the motor moving to
// be sure it is stopped
#define MOTOR_STOPPED_TIME 4000

#define INDEX_MARK_SEARCH_SPEED 100

#define BRAKING_DIST_SIZE 350
const prog_uint16_t braking_distances[BRAKING_DIST_SIZE] = {
0, // 0       
0, // 1       
0, // 2       
8900, // 3       extrapolated
8100, // 4       extrapolated
7300, // 5       extrapolated
6500, // 6       
5800, // 7       
5082, // 8          
4740, // 9          
4108, // 10           
3592, // 11           
3160, // 12           
2904, // 13           
2588, // 14           
2340, // 15           
2128, // 16           
1932, // 17           
1741, // 18           
1656, // 19           
1497, // 20           
1392, // 21           
1257, // 22           
1205, // 23           
1136, // 24           
1092, // 25           
1012, // 26           
937, // 27          
901, // 28          
840, // 29          
781, // 30          
764, // 31          
720, // 32          
689, // 33          
664, // 34     
637, // 35          
613, // 36          
585, // 37          
552, // 38          
540, // 39          
533, // 40          
505, // 41          
481, // 42          
465, // 43          
456, // 44          
441, // 45          
440, // 46          
405, // 47          
392, // 48          
381, // 49          
377, // 50          
373, // 51          
357, // 52          
353, // 53          
333, // 54          
313, // 55          
305, // 56          
301, // 57          
301, // 58          
293, // 59          
289, // 60          
285, // 61          
284, // 62          
273, // 63          
268, // 64          
245, // 65          
245, // 66          
237, // 67          
229, // 68          
229, // 69          
229, // 70          
221, // 71          
217, // 72          
217, // 73          
217, // 74          
213, // 75          
209, // 76          
209, // 77          
208, // 78          
201, // 79          
201, // 80          
197, // 81          
193, // 82          
193, // 83       
181, // 84          
157, // 85          
157, // 86          
157, // 87          
149, // 88          
145, // 89          
145, // 90          
145, // 91          
141, // 92          
141, // 93          
141, // 94          
141, // 95          
137, // 96          
137, // 97          
137, // 98          
137, // 99          
136, // 100           
133, // 101           
133, // 102           
133, // 103           
133, // 104           
133, // 105           
129, // 106           
128, // 107           
125, // 108           
125, // 109           
125, // 110           
125, // 111           
125, // 112           
121, // 113           
121, // 114           
121, // 115           
121, // 116           
117, // 117           
117, // 118           
116, // 119           
113, // 120           
113, // 121           
113, // 122           
105, // 123           
81, // 124          
81, // 125          
77, // 126          
77, // 127          
77, // 128          
77, // 129          
73, // 130          
73, // 131          
73, // 132          
73, // 133          
73, // 134          
73, // 135          
73, // 136          
69, // 137          
69, // 138          
69, // 139          
69, // 140          
69, // 141          
69, // 142          
69, // 143          
69, // 144          
68, // 145          
65, // 146          
65, // 147          
65, // 148          
65, // 149          
65, // 150          
65, // 151          
65, // 152          
65, // 153          
65, // 154          
65, // 155          
61, // 156          
61, // 157          
61, // 158          
61, // 159          
61, // 160          
61, // 161          
61, // 162          
61, // 163          
61, // 164          
61, // 165          
61, // 166          
61, // 167          
61, // 168          
61, // 169          
57, // 170          
57, // 171          
57, // 172          
57, // 173          
57, // 174          
57, // 175          
57, // 176          
57, // 177          
57, // 178       
57, // 179          
57, // 180          
57, // 181          
57, // 182          
57, // 183          
57, // 184          
57, // 185          
57, // 186          
57, // 187          
57, // 188          
57, // 189          
53, // 190          
53, // 191          
53, // 192          
53, // 193          
53, // 194          
53, // 195          
53, // 196          
53, // 197          
53, // 198          
53, // 199          
53, // 200          
53, // 201          
53, // 202          
53, // 203          
53, // 204          
53, // 205          
53, // 206          
53, // 207          
53, // 208          
53, // 209          
53, // 210          
53, // 211          
53, // 212          
49, // 213          
49, // 214          
49, // 215          
49, // 216          
49, // 217          
49, // 218          
49, // 219          
49, // 220          
49, // 221          
49, // 222          
49, // 223          
49, // 224          
49, // 225          
49, // 226          
49, // 227       
49, // 228          
49, // 229          
49, // 230          
49, // 231          
49, // 232          
49, // 233          
49, // 234          
49, // 235          
49, // 236          
49, // 237          
49, // 238          
49, // 239          
49, // 240          
49, // 241          
45, // 242          
45, // 243          
45, // 244          
45, // 245          
45, // 246          
45, // 247          
45, // 248          
45, // 249          
45, // 250          
45, // 251          
45, // 252          
45, // 253          
45, // 254          
45, // 255          
45, // 256          
45, // 257          
45, // 258          
45, // 259          
45, // 260          
45, // 261          
45, // 262          
45, // 263          
45, // 264          
45, // 265          
45, // 266          
45, // 267          
45, // 268          
45, // 269          
45, // 270          
45, // 271          
45, // 272          
45, // 273          
45, // 274          
45, // 275          
45, // 276       
45, // 277          
45, // 278          
45, // 279          
41, // 280          
41, // 281          
41, // 282          
41, // 283          
41, // 284          
41, // 285          
41, // 286          
41, // 287          
41, // 288          
41, // 289          
41, // 290          
41, // 291          
41, // 292          
41, // 293          
41, // 294          
41, // 295          
41, // 296          
41, // 297          
41, // 298          
41, // 299          
41, // 300          
41, // 301          
41, // 302          
41, // 303          
41, // 304          
41, // 305          
41, // 306          
41, // 307          
41, // 308          
41, // 309          
41, // 310          
41, // 311          
41, // 312          
41, // 313          
41, // 314          
41, // 315          
41, // 316          
41, // 317          
41, // 318          
41, // 319          
41, // 320          
41, // 321          
37, // 322          
37, // 323          
37, // 324          
37, // 325       
37, // 326          
37, // 327          
37, // 328
37, // 329
37, // 330
37, // 331
37, // 332
37, // 333
37, // 334
37, // 335
37, // 336
37, // 337
37, // 338
37, // 339
37, // 340
37, // 341
37, // 342
37, // 343
37, // 344
37, // 345
37, // 346
37, // 347
37, // 348
37 // 349
};

// main operating states
typedef enum {
   mcs_stopped,
   mcs_running,
   mcs_seeking_angle,
   mcs_finding_index_mark,
   mcs_braking
   } MotorControllerState;

typedef enum {
   iss_starting_search,
   iss_searching_forward,
   iss_braking_after_forward_search,
   iss_searching_reverse,
   iss_braking_after_reverse_search,
   iss_braking_after_finding_mark
   } IndexSearchState;

typedef struct {
   uint8_t motor_num;   // 1 or 2
   MotorControllerState state;
   int16_t speed;
   uint8_t num_errors;     // accumulated from the optical encoder

   // used during seek to angle
   int32_t target_angle;   
   int16_t seek_speed;      // same magnitude as speed, but in direction
                           // needed to seek target angle

   // used during search for index mark
   IndexSearchState index_search_state;
   int32_t search_start_angle;
   int32_t search_limit_angle;
   } MotorController;

static MotorController controller1;  // uses optical encoder 1 and pwm_a
static MotorController controller2;  // uses optical encoder 2 and pwm_b

static void init_controller (
   const uint8_t motor_num,
   MotorController *controller)
   {
   controller->motor_num = motor_num;
   controller->state = mcs_stopped;
   controller->speed = 64;
   controller->num_errors = 0;
   controller->target_angle = 0;
   controller->seek_speed = 0;
   }

// runs the controller's motor at the currently set speed
static void run_motor (
   const int16_t speed,
   MotorController *controller)
   {
   if (controller->motor_num == 1)
      pwm_a(speed);
   else
      pwm_b(speed);
   }

static void brake_motor (
   MotorController *controller)
   {
   if (controller->motor_num == 1)
      pwm_a_brake();
   else
      pwm_b_brake();
   }

static void set_speed (
   const int16_t new_speed,
   MotorController *controller)
   {
   controller->speed = new_speed;
   if (controller->state == mcs_running)
      run_motor(new_speed, controller);
   }

static void brake_to_stop (
   MotorController *controller)
   {
   if ((controller->state == mcs_running) ||
       (controller->state == mcs_seeking_angle) ||
       (controller->state == mcs_finding_index_mark))
      {
      brake_motor(controller);
      controller->state = mcs_braking;
      }
   }

static void run (
   MotorController *controller)
   {
   controller->state = mcs_running;
   run_motor(controller->speed, controller);
   }

static void seek_angle (
   MotorController *controller)
   {
   int32_t current_angle;
   uint16_t time_at_angle;
   uint8_t num_errors;
   int32_t angle_remaining;

   OpticalEncoder_get_angle_and_time(controller->motor_num,
      &current_angle, &time_at_angle, &num_errors);
   controller->num_errors += num_errors;

   if (controller->seek_speed == 0)
      {  // seek speed not determined yet
      int16_t abs_speed = abs(controller->speed);
      controller->seek_speed =
         (controller->target_angle > current_angle)
         ? abs_speed
         : -abs_speed;
      }

   angle_remaining = (controller->seek_speed > 0)
      ? controller->target_angle - current_angle
      : current_angle - controller->target_angle;
   if (angle_remaining <= 50)
      {  // close enough to target or overshot
      brake_to_stop(controller);
      }
   else  // not there yet
      {
      // get braking distance at the current speed
      uint16_t braking_dist = (time_at_angle < BRAKING_DIST_SIZE)
         ? pgm_read_word_near(
            &braking_distances[time_at_angle])
         : 0;
      if (angle_remaining <= braking_dist)
         {  // at this speed we are within braking distance
         brake_motor(controller); // brake now
         }
      else
         {
         run_motor(controller->seek_speed, controller);
         }
      }

   }

static void start_seeking_angle (
   const int32_t new_angle,
   MotorController *controller)
   {
   controller->state = mcs_seeking_angle;
   controller->target_angle = new_angle;
   controller->seek_speed = 0;   // cause seek_angle to compute seek speed
   seek_angle(controller);
   }

// state machine for finding the optical switch index mark
static void find_index_mark (
   MotorController *controller)
   {
   switch (controller->index_search_state) {
      case iss_starting_search :
         {
         int32_t current_angle;
         uint16_t time_at_angle;
         uint8_t num_errors;

         // instruct optical encoder to reset angle when the index mark
         // is detected
         OpticalEncoder_enable_reset_at_index_mark(controller->motor_num);
         
         // set a limit on how far to search
         OpticalEncoder_get_angle_and_time(controller->motor_num,
            &current_angle, &time_at_angle, &num_errors);
         controller->num_errors += num_errors;
         controller->search_limit_angle = current_angle + 6000;
         controller->search_start_angle = current_angle;

         // run the motor and update search state
         run_motor(INDEX_MARK_SEARCH_SPEED, controller);
         controller->index_search_state = iss_searching_forward;
         }
         break;
      case iss_searching_forward :
         {
         // search forward until the index mark is found or we have
         // reached the search limit
         if (OpticalEncoder_index_mark_detected(controller->motor_num))
            {  // we have found the mark
            brake_motor(controller);
            controller->index_search_state = iss_braking_after_finding_mark;
            }
         else    // index mark not detected yet
            {
            // check if we have reached the limit
            int32_t current_angle;
            uint16_t time_at_angle;
            uint8_t num_errors;
            OpticalEncoder_get_angle_and_time(controller->motor_num,
               &current_angle, &time_at_angle, &num_errors);
            controller->num_errors += num_errors;
            if (current_angle >= controller->search_limit_angle)
               {  // we have reached the limit
               brake_motor(controller);
               controller->index_search_state = iss_braking_after_forward_search;
               }
            }
         }
         break;
      case iss_braking_after_forward_search :
         {
         int32_t current_angle;
         uint16_t time_at_angle;
         uint8_t num_errors;
         OpticalEncoder_get_angle_and_time (controller->motor_num,
            &current_angle, &time_at_angle, &num_errors);
         controller->num_errors += num_errors;
         if (time_at_angle >= MOTOR_STOPPED_TIME)
            {  // motor has come to a stop
            // begin reverse search
            // set a limit on how far to search
            controller->search_limit_angle = current_angle - 12000;

            // run the motor and update search state
            run_motor(-INDEX_MARK_SEARCH_SPEED, controller);
            controller->index_search_state = iss_searching_reverse;
            }
         }
         break;
      case iss_searching_reverse :
         // search reverse until the index mark is found or we have
         // reached the search limit
         if (OpticalEncoder_index_mark_detected(controller->motor_num))
            {  // we have found the mark
            brake_motor(controller);
            controller->index_search_state = iss_braking_after_finding_mark;
            }
         else    // index mark not detected yet
            {
            // check if we have reached the limit
            int32_t current_angle;
            uint16_t time_at_angle;
            uint8_t num_errors;
            OpticalEncoder_get_angle_and_time(controller->motor_num,
               &current_angle, &time_at_angle, &num_errors);
            controller->num_errors += num_errors;
            if (current_angle <= controller->search_limit_angle)
               {  // we have reached the limit without finding the mark
               brake_motor(controller);
               controller->index_search_state = iss_braking_after_reverse_search;
               }
            }
         break;
      case iss_braking_after_reverse_search :
         {  // if we get to this state we failed to find the index mark
         int32_t current_angle;
         uint16_t time_at_angle;
         uint8_t num_errors;
         OpticalEncoder_get_angle_and_time (controller->motor_num,
            &current_angle, &time_at_angle, &num_errors);
         controller->num_errors += num_errors;
         if (time_at_angle >= MOTOR_STOPPED_TIME)
            {  // motor has come to a stop
            // log error and begin return to start position
            // controller->num_errors = 1;
            start_seeking_angle(controller->search_start_angle, controller);
            }
         }
         break;
      case iss_braking_after_finding_mark :
         {  // if we get to this state we found the index mark
         int32_t current_angle;
         uint16_t time_at_angle;
         uint8_t num_errors;
         OpticalEncoder_get_angle_and_time (controller->motor_num,
            &current_angle, &time_at_angle, &num_errors);
         controller->num_errors += num_errors;
         if (time_at_angle >= MOTOR_STOPPED_TIME)
            {  // motor has come to a stop
            // begin moving to zero
            start_seeking_angle(0, controller);
            }
         }
         break;
      }
   }

#if 0

static uint8_t angle_buffer[32];
static uint8_t comment[] = ", // ";
static uint8_t done_flag;

static int16_t measured_breaking_distances[BRAKING_DIST_SIZE];

void measure_breaking_distances (void)
   {
   uint16_t i;
   LaserScannerControllerState scanner_state;
   uint8_t encoder_bits;
   EncoderBitAnalysisState encoder_state;
   uint16_t last_time_at_angle = 0;

   initialize_scanner_controller(&scanner_state);

   for (i = 0; i < BRAKING_DIST_SIZE; ++i)
      measured_breaking_distances[i] = 0;

   // rev up to full speed
   motor_a_fwd();
   do
      {
      update_scanner_controller(&scanner_state, &encoder_state);
      }
   while (scanner_state.current_angle < 15000);

   // brake and capture angles at speeds while decelarating
   motor_a_brake();
   do
      {
      update_scanner_controller(&scanner_state, &encoder_state);
      if ((scanner_state.time_at_angle < BRAKING_DIST_SIZE) &&
          (scanner_state.time_at_angle > last_time_at_angle))
         {
         measured_breaking_distances[scanner_state.time_at_angle] =
            scanner_state.current_angle;
         last_time_at_angle = scanner_state.time_at_angle;
         }
      }
   while (scanner_state.time_at_angle < MOTOR_STOPPED_TIME);

   // normalize distances to the angle we ended at
   for (i = 0; i < BRAKING_DIST_SIZE; ++i)
      if (measured_breaking_distances[i] != 0)
         measured_breaking_distances[i] =
            scanner_state.current_angle - measured_breaking_distances[i];

   // report distances
   for (i = 0; i < BRAKING_DIST_SIZE; ++i)
      {
      itoa((int)measured_breaking_distances[i], angle_buffer, 10);
   	uartSendBuffer(angle_buffer,strlen(angle_buffer),&done_flag);
   	uartWait(&done_flag);
   	uartSendBuffer(comment ,strlen(comment),&done_flag);
   	uartWait(&done_flag);
      itoa((int)i, angle_buffer, 10);
   	uartSendBuffer(angle_buffer,strlen(angle_buffer),&done_flag);
   	uartWait(&done_flag);
    	uartSendByte(0x0d);		// cr
   	uartSendByte(0x0A);		// lf
      delayms(50);
      }
   }
#endif

static void update_controller (
   MotorController *controller,
   Bool *motor_has_just_stopped)
   {
   *motor_has_just_stopped = false;

   switch (controller->state) {
      case mcs_stopped :
         break;
      case mcs_running :
         break;
      case mcs_seeking_angle :
         seek_angle(controller);
         break;
      case mcs_finding_index_mark :
         find_index_mark(controller);
         break;
      case mcs_braking :
         {
         int32_t current_angle;
         uint16_t time_at_angle;
         uint8_t num_errors;
         // if motor is now stopped then return position and return to
         // the stopped state
         OpticalEncoder_get_angle_and_time (controller->motor_num,
            &current_angle, &time_at_angle, &num_errors);
         controller->num_errors += num_errors;
         if (time_at_angle >= MOTOR_STOPPED_TIME)
            {
            *motor_has_just_stopped = true;
            controller->state = mcs_stopped;
            }
         }
         break;
      }
   }

void MotorController_init ()
   {
	OpticalEncoder_init();
   pwm_init();

   init_controller(1, &controller1);
   init_controller(2, &controller2);
   }

void MotorController_update (
   const uint8_t which_motor,
   Bool *motor_has_just_stopped)
   {
   update_controller(
      (which_motor == 1) ? &controller1 : &controller2,
      motor_has_just_stopped);
   }

void MotorController_get_current_angle (
   const uint8_t which_motor,
   int32_t *current_angle,
   uint8_t *num_errors)
   {
   uint8_t new_num_errors;
   uint16_t time_at_angle;
   MotorController *controller =
      (which_motor == 1) ? &controller1 : &controller2;
   OpticalEncoder_get_angle_and_time(which_motor,
      current_angle, &time_at_angle, &new_num_errors);
   *num_errors = controller->num_errors + new_num_errors;
   controller->num_errors = 0;
   }

void MotorController_set_speed (
   const uint8_t which_motor,
   const int16_t new_speed)
   {
   set_speed(new_speed, (which_motor == 1) ? &controller1 : &controller2);
   }

void MotorController_move_to_angle (
   const uint8_t which_motor,
   const int32_t new_angle)
   {
   MotorController *controller =
      (which_motor == 1) ? &controller1 : &controller2;
   start_seeking_angle(new_angle, controller);
   }

void MotorController_run (
   const uint8_t which_motor)
   {
   run((which_motor == 1) ? &controller1 : &controller2);
   }

void MotorController_brake_to_stop (
   const uint8_t which_motor)
   {
   brake_to_stop((which_motor == 1) ? &controller1 : &controller2);
   }

void MotorController_find_index_mark (
   const uint8_t which_motor)
   {
   MotorController *controller =
      (which_motor == 1) ? &controller1 : &controller2;
   controller->state = mcs_finding_index_mark;
   controller->index_search_state = iss_starting_search;
   }

