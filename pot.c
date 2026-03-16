#include "pot.h"
#include "adc.h"
#include "stm32f4xx_hal.h"

//Cola para mensajes del volumen
osThreadId_t tid_POT;

void Thread_POT(void *argument);

// Variables del ADC
static ADC_HandleTypeDef adchandle;
float aux_debug = 0.0f; //variable para ver watches

float last_voltage = -1.0f;


int Init_POT(void) {

    // 2. Crear el hilo
  tid_POT = osThreadNew(Thread_POT, NULL, NULL);
  if (tid_POT == NULL) {
    return(-1);
  }

  return(0);
}

void Thread_POT(void *argument) {
    float current_voltage = 0.0f;
   // static float last_voltage = 0.0f;
    const float UMBRAL = 0.05f; // Margen de cambio (100mV) para evitar ruido

    // Funciones del adc

    // 1. Configurar los pines (PC0 para POT_2 según tabla de conexiones)
    ADC1_pins_F429ZI_config();

    // 2. Inicializar el ADC1
    ADC_Init_Single_Conversion(&adchandle, ADC1);

    while (1) {

        // 3. Leer el voltaje
        current_voltage = ADC_getVoltage(&adchandle, 10);
                aux_debug = current_voltage;

        // 4. Comprobar si ha cambiado significativamente (Hysteresis)
        // Si la diferencia en valor absoluto es mayor que el umbral mandar mensaje
        if (((current_voltage - last_voltage) > UMBRAL)||((current_voltage - last_voltage)<-UMBRAL)) {

            // Enviar el nuevo valor a la cola
            osMessageQueuePut(mid_Volumen, &current_voltage, 0, 0);

            // Actualizamos el último valor
            last_voltage = current_voltage;
        }
        osDelay(100);
    }
}