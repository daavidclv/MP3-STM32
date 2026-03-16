#ifndef MP3_H
#define MP3_H

#include <stdint.h>
#include "cmsis_os2.h" // Necesario si expones tipos del RTOS, aunque aquí no es estrictamente necesario si encapsulas bien.



void MP3_Init(void);
int Init_Thread_MP3 (void);
void MP3_Send_Command(uint8_t *cmd);
extern osMessageQueueId_t mid_mp3;

int MP3_RX_Init (void);
#endif
