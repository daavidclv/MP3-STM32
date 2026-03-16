
#ifndef CLOCK_H
#define CLOCK_H
#include "stm32f4xx_hal.h"
#include "cmsis_os2.h"
extern uint8_t segundos;
extern uint8_t minutos;
extern uint8_t horas;
int Init_Thread_hora (void);
#endif

