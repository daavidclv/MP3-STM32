#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "rgb.h"
/*----------------------------------------------------------------------------
 *      Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/
 
static osThreadId_t tid_Thread;                        // thread id
static GPIO_InitTypeDef GPIO_InitStruct;
static uint8_t led = 4;
void Thread_RGB (void *argument);                   // thread function
void initleds(void);
void l_verde(void);
void l_rojo(void);
void l_azul(void);
void l_off(void);

int Init_Thread_RGB (void) {
 
  tid_Thread = osThreadNew(Thread_RGB, NULL, NULL);
  if (tid_Thread == NULL) {
    return(-1);
  }
 
  return(0);
}

//Logica para  el parpadeo si esta bien o mal  4Hz == 125|| 1Hz==500
 void Thread_RGB (void *argument) {
 uint8_t nuevo_mensaje;
 initleds();
   
while (1) {
        // 1. REVISAR SI HAY CAMBIO DE ORDEN (Timeout 0U = No esperar)
        // Si hay mensaje, actualizamos la variable de estado. Si no, seguimos con la que teníamos.
        if (osMessageQueueGet(leds, &nuevo_mensaje, NULL, 0U) == osOK){
            led = nuevo_mensaje;
        }

        // 2. EJECUTAR EL COMPORTAMIENTO CONTINUO SEGÚN EL ESTADO
        if (led == 1){ 
            // VERDE (Reproduciendo - 4Hz)
            l_verde();
            osDelay(125);
            l_off();
            osDelay(125);

        } else if (led == 2){ 
            // AZUL (Pausa - 1Hz)
            l_azul();
            osDelay(500);
            l_off();
            osDelay(500);
                            
        } else if (led == 0){ 
            // ROJO (Error/No SD - 5Hz)
            l_rojo();
            osDelay(100); // 100ms ON + 100ms OFF = 200ms periodo (5Hz)
            l_off();
            osDelay(100);

        } else { 
            // APAGADO (Caso 4 o cualquier otro)
            l_off();
            // IMPORTANTE: Un pequeño delay cuando está apagado para liberar la CPU
            // Si no pones esto, el hilo gira a tope consumiendo recursos inútilmente.
            osDelay(100); 
        }
    }
}
/*void Thread_RGB (void *argument) {
 initleds();
  while (1) {
    if (osMessageQueueGet(leds, &led, NULL, osWaitForever) == osOK){
			if (led == 1){
				l_verde();
			}else if (led == 2){
			l_rojo();
			}else if(led == 3){
				l_azul();
			}else{
			
			}
		
		} // Insert thread code here...
    osThreadYield();                            // suspend thread
  }
}
*/

void initleds(void){
	__HAL_RCC_GPIOD_CLK_ENABLE();
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	
	GPIO_InitStruct.Pin = GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13 ;
	HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}
void l_verde(void){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}
void l_rojo(void){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
}
void l_azul(void){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}
void l_off(void){
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
}