#include "joystick.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

/*----------------------------------------------------------------------------
 * VARIABLES GLOBALES
 *---------------------------------------------------------------------------*/

osTimerId_t swtim_id;          // Timer Rebote (One-Shot)
osTimerId_t timpuls_id;        // Timer Muestreo (Periodic)
osThreadId_t tid_Thread_joy;       // Thread joystick

osMessageQueueId_t colaJoystickgesto;      
osMessageQueueId_t colaJoystickpulsacion; 

static int boton_presionado = 0;
//nuevo
static uint8_t gesto_detectado = 0;

// Variables auxiliares 
static GPIO_TypeDef* puerto_actual = NULL;
static uint16_t pin_actual = 0;
static int contador_tiempo = 0;

/* --- VARIABLES DE DEBUG (Para ver en Watch 1) --- */
volatile int DEBUG_ISR_COUNT = 0;     // ¿Cuántas veces salta la interrupción?
volatile int DEBUG_PIN_DETECTADO = 0; // ¿Qué pin ha leído el HAL? (10, 11, 12...)
volatile int DEBUG_GESTO_ID = 0;      // ¿Qué gesto ha decidido el código? (1, 2, 4...)


/*----------------------------------------------------------------------------
 * PROTOTIPOS
 *---------------------------------------------------------------------------*/
 void Timer1_Callback(void  *arg);
 void Timer2_Callback(void  *arg);
 void TimerGestos_Callback(void *arg);
 void Init_Joystick(void); 

/*----------------------------------------------------------------------------
 * INICIALIZACIÓN DEL HILO
 *---------------------------------------------------------------------------*/
int Init_Thread_Joystick(void)
{
    // 1. Crear Colas
    colaJoystickgesto     = osMessageQueueNew(10, sizeof(uint8_t), NULL);
    colaJoystickpulsacion = osMessageQueueNew(10, sizeof(uint8_t), NULL);

    // 2. Crear Timers
    swtim_id   = osTimerNew(Timer1_Callback, osTimerOnce, NULL, NULL);
    timpuls_id = osTimerNew(Timer2_Callback, osTimerPeriodic, NULL, NULL);

    // 3. Crear Hilo
    tid_Thread_joy = osThreadNew(Thread_joy, NULL, NULL);
    if (!tid_Thread_joy) return -1;

    // 4. Inicializar Hardware (IMPORTANTE: Después de crear el hilo)
    Init_Joystick(); 

    return 0;
}

/*----------------------------------------------------------------------------
 * THREAD JOYSTICK
 *---------------------------------------------------------------------------*/
void Thread_joy(void *arg)
{
    while (1)
    {
        // Espera a que la interrupción (en stm32f4xx_it.c) mande la señal 0x01
        osThreadFlagsWait(0x01, osFlagsWaitAny, osWaitForever);

        // Inicia el timer de rebote (50ms)
        osTimerStart(swtim_id, 50); 
    }
}

/*----------------------------------------------------------------------------
 * CALLBACK GESTOS (Identifica botón)
 *---------------------------------------------------------------------------*/
void TimerGestos_Callback(void *arg)
{
    uint8_t gesto = 0;
    
		DEBUG_PIN_DETECTADO = 0;
	
	
    puerto_actual = NULL; 
    pin_actual = 0;

    // CAMBIO CRÍTICO: Usamos GPIO_PIN_SET (1) porque tu hardware es Activo Alto
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10) == GPIO_PIN_SET) { /*ARRIBA*/
        gesto = 1; 
			  puerto_actual = GPIOB; 
			  pin_actual = GPIO_PIN_10; 
				DEBUG_PIN_DETECTADO = 10; // DEBUG
    }
    if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_11) == GPIO_PIN_SET) { /*DERECHA*/
        gesto = 2; 
				puerto_actual = GPIOB; 
				pin_actual = GPIO_PIN_11; 
				DEBUG_PIN_DETECTADO = 11; // DEBUG
    }
     if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_12) == GPIO_PIN_SET) { /*ABAJO*/
        gesto = 4; 
				puerto_actual = GPIOE; 
				pin_actual = GPIO_PIN_12; 
				DEBUG_PIN_DETECTADO = 12; // DEBUG
    }
     if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_14) == GPIO_PIN_SET) { /*IZQUIERDA*/
        gesto = 8; 
				puerto_actual = GPIOE; 
				pin_actual = GPIO_PIN_14; 
				DEBUG_PIN_DETECTADO = 14; // DEBUG
    }
     if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_15) == GPIO_PIN_SET) {/*CENTRO*/ 
        gesto = 16; 
				puerto_actual = GPIOE; 
				pin_actual = GPIO_PIN_15; 
				DEBUG_PIN_DETECTADO = 15; // DEBUG
    }
		DEBUG_GESTO_ID = gesto;
		
    if (gesto != 0) {
				gesto_detectado = gesto;
        //osMessageQueuePut(colaJoystickgesto, &gesto, 0, 0); por esto no iba el joystick!!!!
        boton_presionado = 1; 
    } else {
        boton_presionado = 0; 
    }
}

/*----------------------------------------------------------------------------
 * CALLBACK TIMER 1 (Rebote terminado)
 *---------------------------------------------------------------------------*/
void Timer1_Callback(void *arg)
{
    // Detectar gesto
    TimerGestos_Callback(NULL);

    // Si detectamos pulsación, arrancamos el vigilante de tiempo
    if (boton_presionado == 1) {
        contador_tiempo = 0;
        osTimerStart(timpuls_id, 100); 
    }
}

/*----------------------------------------------------------------------------
 * CALLBACK TIMER 2 (Vigilante Periódico)
 *---------------------------------------------------------------------------*/
void Timer2_Callback(void *arg)
{
    uint8_t tipo;
		uint8_t g_envio = gesto_detectado;
    // CAMBIO CRÍTICO: Chequeamos si sigue en SET (Alto)
    if (puerto_actual != NULL && HAL_GPIO_ReadPin(puerto_actual, pin_actual) == GPIO_PIN_SET) {
        
        contador_tiempo += 100;

        // Si llega a 1000ms -> Larga
        if (contador_tiempo >= 1000) {
            tipo = 1; // Larga
					//nuevo
						osMessageQueuePut(colaJoystickgesto, &g_envio, 0, 0);
            osMessageQueuePut(colaJoystickpulsacion, &tipo, 0, 0);
            
            osTimerStop(timpuls_id); 
            boton_presionado = 0;
        }
    } 
    else {
        // SE SOLTÓ EL BOTÓN (Ahora lee LOW/RESET)
        
        if (contador_tiempo < 3000) {
            tipo = 0; // Corta
					//nuevo
						osMessageQueuePut(colaJoystickgesto, &g_envio, 0, 0);
            osMessageQueuePut(colaJoystickpulsacion, &tipo, 0, 0);
        }
        
        osTimerStop(timpuls_id);
        boton_presionado = 0;
        puerto_actual = NULL;
    }
}

/*----------------------------------------------------------------------------
 * HARDWARE (INIT)
 *---------------------------------------------------------------------------*/
// Esta función debe llamarse DESPUÉS de crear el hilo, no antes.
void Init_Joystick(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOE_CLK_ENABLE();

    // CAMBIO CRÍTICO: RISING para detectar cuando pulsas (sube a 3.3V)
    GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING; 
    // CAMBIO CRÍTICO: PULLDOWN para que esté en 0V si no tocas nada
    GPIO_InitStruct.Pull  = GPIO_PULLDOWN;       
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;

    GPIO_InitStruct.Pin = GPIO_PIN_10 | GPIO_PIN_11;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_12 | GPIO_PIN_14 | GPIO_PIN_15;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    // Limpiar flags NO AFECTA POR AHORA
    /*__HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_10);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_11);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_12);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_14);
    __HAL_GPIO_EXTI_CLEAR_IT(GPIO_PIN_15);*/

    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}
