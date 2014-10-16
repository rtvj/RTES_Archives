#ifndef _Race_Ctl_H_
#define _Race_Ctl_H_

/* 24+8+6 = 38 bytes */
struct race_state
{
  /* Translational */
  int pos;
  int vel;
  int accel;

  /* Rotational */
  int yaw;
  int yaw_rate;
  int yaw_accel;

  /* Commands */
  int direction;
  unsigned short right_servo_cmd;
  unsigned short left_servo_cmd;
  unsigned short tilt_servo_cmd;
  unsigned short pan_servo_cmd;

  /* Target Dimensions */
  int target;
  unsigned short target_size;
  unsigned short target_x;
  unsigned short target_y;

};

void race_ctl_cycle(void);

void yaw_ctl_cycle(void);

#endif
