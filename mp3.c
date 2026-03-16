#include "mp3.h"
#include "cmsis_os2.h"
#include "Driver_USART.h"
#include "mp3.h"
#include <string.h>
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_MP3;                        // thread id
extern ARM_DRIVER_USART Driver_USART2;
void Thread_MP3 (void *argument);                   // thread function
void myUSART_MP3_callback(uint32_t event);
static uint8_t rx_buffer[10];
static volatile uint8_t tarjeta_insertada=1;
int a = 0;
const osThreadAttr_t thread1_attr = {
  .stack_size = 1024                            // Creamos el hilo con menos memoria
};
int Init_Thread_MP3 (void) {
  tid_Thread_MP3 = osThreadNew(Thread_MP3, NULL, &thread1_attr);
  if (tid_Thread_MP3 == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_MP3 (void *argument) {
	uint8_t cmd_buffer[8];
	MP3_Init();

  while (1) {

    if(osMessageQueueGet(mid_mp3, &cmd_buffer, NULL, osWaitForever)== osOK){

						Driver_USART2.Send(cmd_buffer, 8);
            osThreadFlagsWait(0x032, osFlagsWaitAny, osWaitForever);		
		}
    osThreadYield();                            // suspend thread
  }
}
void MP3_Init(void) {
    

    // Inicializar el Driver UART seleccionado
    Driver_USART2.Initialize(myUSART_MP3_callback);
    Driver_USART2.PowerControl(ARM_POWER_FULL);

    // Configuración OBLIGATORIA según Datasheet Catalex[cite: 42, 50]:
    // 9600 Baudios, 8 Data bits, No Parity, 1 Stop bit
    Driver_USART2.Control(ARM_USART_MODE_ASYNCHRONOUS |
                          ARM_USART_DATA_BITS_8 |
                          ARM_USART_PARITY_NONE |
                          ARM_USART_STOP_BITS_1 |
                          ARM_USART_FLOW_CONTROL_NONE, 9600);

    // Activar líneas TX y RX
    Driver_USART2.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART2.Control(ARM_USART_CONTROL_RX, 1);
}







/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
osThreadId_t tid_Thread_MP3_Rx;                        // thread id
 
void Thread_MP3_RX (void *argument);                   // thread function
osMessageQueueId_t mid_mp3_rx;
 
int MP3_RX_Init (void) {
 
  tid_Thread_MP3_Rx = osThreadNew(Thread_MP3_RX, NULL, &thread1_attr);
  if (tid_Thread_MP3_Rx == NULL) {
    return(-1);
  }
 
  return(0);
}
 
void Thread_MP3_RX (void *argument) {
          uint8_t recibido = 0;
         uint8_t codigo_evento;
         MP3_Init();
         mid_mp3_rx         = osMessageQueueNew(8, 8, NULL);
         
  while (1) {
// 1. DORMIR hasta que el Callback diga "Han llegado datos" (Flag 0x02)
    
        Driver_USART2.Receive(rx_buffer, 10);
        osThreadFlagsWait(0x08, osFlagsWaitAny, osWaitForever);
        codigo_evento = rx_buffer[3];
        if (codigo_evento == 0x3A) {
         recibido = 1;
         osMessageQueuePut(mid_mp3_rx, &recibido, 0, osWaitForever);
        }
        else if (codigo_evento == 0x3B) {
         recibido = 2;
         osMessageQueuePut(mid_mp3_rx, &recibido, 0, osWaitForever);
        }
        else if (codigo_evento == 0x3D) { // Fin de canción 
          recibido = 3; //
          osMessageQueuePut(mid_mp3_rx, &recibido, 0, 0);
}
        
        // 3. REARMAR LA ESCUCHA (CRÍTICO)
        // El driver deja de escuchar tras recibir. Hay que decirle "escucha de nuevo".

    }
  }
/* --- CALLBACK (El Semáforo) --- */
void myUSART_MP3_callback(uint32_t event) {
    
    // Caso A: Se completó un ENVÍO -> Despierta al hilo TX
// Si termina de ENVIAR -> Despierta a TX
if (event & ARM_USART_EVENT_SEND_COMPLETE)    {

osThreadFlagsSet(tid_Thread_MP3, 0x32);}
// Si termina de RECIBIR -> Despierta a RX
if (event & ARM_USART_EVENT_RECEIVE_COMPLETE) {

osThreadFlagsSet(tid_Thread_MP3_Rx, 0x08);

a = 3;}
}

