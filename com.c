#include "com.h"
#include "cmsis_os2.h"
#include "Driver_USART.h"
#include <string.h>
#include <stdio.h>


// RECURSOS 
osMessageQueueId_t colaMensaje;
osMessageQueueId_t colaRx; // <--- FALTABA ESTA VARIABLE (Por eso el error L6218E)
osThreadId_t tid_uart;

// DRIVER 

extern ARM_DRIVER_USART Driver_USART3;

// Variable auxiliar para recibir dato
static char rx_char; 

// Callback
void myUSART_callback(uint32_t event) {
  if(event & ARM_USART_EVENT_SEND_COMPLETE){
    osThreadFlagsSet(tid_uart, 0x01);
  }
  // Añadimos la recepción para que funcione el control desde PC
  if(event & ARM_USART_EVENT_RECEIVE_COMPLETE){
    osMessageQueuePut(colaRx, &rx_char, 0, 0U);
    Driver_USART3.Receive(&rx_char, 1); // Volver a escuchar
  }
}

//Hilo del UART-> espreramos mensajes y enviamos
void uart_msg(void *argument) {
    char mensaje[64]; 

    while(1) {
        
        if (osMessageQueueGet(colaMensaje, &mensaje, NULL, osWaitForever) == osOK) {
            Driver_USART3.Send(mensaje, strlen(mensaje));
            osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);
        }
    }
}

// Inicialización
void UART_Init(void) {
  
    colaMensaje = osMessageQueueNew(30, 64, NULL); 
    colaRx = osMessageQueueNew(10, sizeof(char), NULL); // <--- FALTABA INICIALIZARLA

    Driver_USART3.Initialize(myUSART_callback);
    Driver_USART3.PowerControl(ARM_POWER_FULL);
    // 9600 BAUDIOS
    Driver_USART3.Control(ARM_USART_MODE_ASYNCHRONOUS |
                           ARM_USART_DATA_BITS_8 |
                           ARM_USART_PARITY_NONE |
                           ARM_USART_STOP_BITS_1 |
                           ARM_USART_FLOW_CONTROL_NONE, 115200);
  
  // Activa los pines TX y RX
    Driver_USART3.Control(ARM_USART_CONTROL_TX, 1);
    Driver_USART3.Control(ARM_USART_CONTROL_RX, 1);
    
    // Empezar a escuchar (Importante para que funcione la recepción)
    Driver_USART3.Receive(&rx_char, 1);

  // Crea el hilo que se encargará de gestionar los envíos
    tid_uart = osThreadNew(uart_msg, NULL, NULL);
}

//METEMOS EL MESNAJE
void UART_MSG(char *mensaje) {
    // IMPORTANTE: Ponemos 0U en vez de osWaitForever para que no bloquee tu menú
    osMessageQueuePut(colaMensaje, mensaje, 0, 0U); 
}