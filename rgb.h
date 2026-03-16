#ifndef RGB_H
#define RGB_H
#include "cmsis_os2.h"   
#include "RTE_Components.h"
#include "stm32f4xx_hal.h"
int Init_Thread_RGB (void);
extern osMessageQueueId_t leds;
#endif
