#ifndef JOYSTICK_H
#define JOYSTICK_H
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"


/*----------------------------------------------------------------------------
 *     									FUNCIONES/variables HILO
 *---------------------------------------------------------------------------*/
extern osThreadId_t tid_Thread_joy;
extern osTimerId_t swtim_id;
extern osTimerId_t timpuls_id;
extern osMessageQueueId_t colaJoystickgesto;
extern osMessageQueueId_t colaJoystickpulsacion;

void Thread_joy (void *arg);
void TimerGestos_Callback (void  *arg);
void Timer1_Callback(void  *arg);
void Timer2_Callback(void  *arg);

/*----------------------------------------------------------------------------
 *     									FUNCIONES/variables JOYSTICK
 *---------------------------------------------------------------------------*/
int Init_Thread_Joystick(void);

static int cnt;


#endif
