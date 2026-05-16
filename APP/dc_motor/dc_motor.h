/*
 * dc_motor.h
 */

#ifndef DC_MOTOR_H_
#define DC_MOTOR_H_

#include "DSP2833x_Device.h"
#include "DSP2833x_Examples.h"

#define MOTOR_DIR_STOP     0
#define MOTOR_DIR_FORWARD  1
#define MOTOR_DIR_REVERSE  2

#define MOTOR_SPEED_MAX    7

void DC_Motor_Init(void);
void DCMotor_ePWM3_Init(void);
void DCMotor_Set(Uint16 enable, Uint16 dir, Uint16 speed);
void DCMotor_Stop(void);

#endif
