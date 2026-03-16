#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "clock.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_hora;                        // thread id
 
void Thread_hora (void *argument);                   // thread function
 void incrementarsegundo(void);
int Init_Thread_hora (void) {
 const osThreadAttr_t thread1_attrc = {
  .stack_size = 1024                            // Creamos el hilo con menos memoria
};
  tid_Thread_hora = osThreadNew(Thread_hora, NULL, &thread1_attrc);
  if (tid_Thread_hora == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_hora (void *argument) {
 
  while (1) {
    incrementarsegundo(); // Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}

void incrementarsegundo(void){
  osDelay(1000);
  segundos=segundos+1;
  if(segundos==60){
    segundos=0;
    minutos++;
  }
  if(minutos==60){
    minutos=0;
    horas++;
  }
}
