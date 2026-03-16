#ifndef PWM_H
#define PWM_H
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"
int Init_ThreadPWM (void);
extern osThreadId_t tid_Thread_PWM;

#define BIP_CORTO 1
#define BIP_DOBLE 2
#endif
