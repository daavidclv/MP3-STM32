#ifndef LCD_H
#define LCD_H
#include "stm32f4xx_hal.h"
#include "Driver_SPI.h"
#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "RTE_Components.h"

int Init_Thread_LCD (void);
extern osMessageQueueId_t colamensaje;
extern osMessageQueueId_t colaLCD_L1;
extern osMessageQueueId_t colaLCD_L2;
/*
Inicializar el TIM7
Inicializar el SPI(RTE_Device MOSI PA7;SCK PA5)
Primero funcion Iniciacion_LCD()
Segundo 
*/
#endif
