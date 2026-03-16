#ifndef TEMP_H
#define TEMP_H
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "stm32f4xx_hal.h"
int Init_Thread_temp (void);
extern osMessageQueueId_t colatemp;
#endif
