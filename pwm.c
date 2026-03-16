#include "PWM.h"
#include "cmsis_os2.h"
#include "stm32f4xx_hal.h"

// --- DEFINICIÓN DE COMANDOS PARA LA COLA ---
#define BIP_CORTO 1
#define BIP_DOBLE 2

#define PERIODO 1000

/*----------------------------------------------------------------------------
 * VARIABLES GLOBALES
 *---------------------------------------------------------------------------*/
osThreadId_t tid_Thread_PWM;                        // Thread ID
osMessageQueueId_t mid_PWM;                         // NUEVO: ID de la Cola PWM

static TIM_HandleTypeDef htim1;

/*----------------------------------------------------------------------------
 * PROTOTIPOS
 *---------------------------------------------------------------------------*/
void Thread (void *argument);                   // thread function
void Init_TIM (uint32_t periodo);

/*----------------------------------------------------------------------------
 * INICIALIZACIÓN DEL HILO Y LA COLA
 *---------------------------------------------------------------------------*/
int Init_ThreadPWM (void) {

  mid_PWM = osMessageQueueNew(5, sizeof(uint8_t), NULL);
  if (mid_PWM == NULL) {
    return(-1); // Error al crear la cola
  }

  tid_Thread_PWM = osThreadNew(Thread, NULL, NULL);
  if (tid_Thread_PWM == NULL) {
    return(-1);
  }

  return(0);
}

/*----------------------------------------------------------------------------
 * HILO PRINCIPAL DEL PWM
 *---------------------------------------------------------------------------*/
void Thread (void *argument){

  Init_TIM(PERIODO);
  uint8_t comando_recibido = 0; // Variable para guardar lo que sacamos de la cola

  while (1) {

        // Esperamos a recibir un mensaje en la cola (bloqueante)
        if (osMessageQueueGet(mid_PWM, &comando_recibido, NULL, osWaitForever) == osOK) {

            // --- OPCIÓN A: BIP CORTO (Comando 1) ---
            if (comando_recibido == 1) {
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);
                osDelay(200); // Suena durante 200ms
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
            }

            // --- OPCIÓN B: BIP DOBLE (Comando 2) ---
            else if (comando_recibido == 2) {
              // --- Jingle bells (Mi, Mi, Mi) ---
                // Mi (659Hz) -> Periodo aprox 1516
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50); // Pausa breve

                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(400); // Nota larga
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // --- Jingle bells (Mi, Mi, Mi) ---
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(400); 
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // --- Jingle All The Way (Mi, Sol, Do, Re, Mi) ---
                
                // Mi
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // Sol (784Hz) -> Periodo 1274
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1274);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 637);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // Do (523Hz) -> Periodo 1911
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1911);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 955);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // Re (587Hz) -> Periodo 1702
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1702);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 851);
                osDelay(200);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);

                // Mi (Larga)
                __HAL_TIM_SET_AUTORELOAD(&htim1, 1516);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 758);
                osDelay(600);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0); osDelay(50);
                
                // --- FIN DE LA MELODÍA ---
                // Asegurar silencio total al terminar
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
               /* // Primer pitido
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);
                osDelay(100);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);

                // Silencio entre medias
                osDelay(100);

                // Segundo pitido
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 500);
                osDelay(100);
                __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);*/
            }
        }
  }
}

/*----------------------------------------------------------------------------
 * INICIALIZACIÓN DEL TIMER (HARDWARE)
 *---------------------------------------------------------------------------*/
void Init_TIM (uint32_t periodo){
  __HAL_RCC_TIM1_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();

  GPIO_InitTypeDef GPIO_InitStruct = {0};
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 167;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = periodo;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  HAL_TIM_PWM_Init(&htim1);

  TIM_OC_InitTypeDef sConfigOC = {0};
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCIDLESTATE_RESET;

  HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

  HAL_TIMEx_PWMN_Start(&htim1, TIM_CHANNEL_1);
  __HAL_TIM_MOE_ENABLE(&htim1);
}