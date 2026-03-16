
#ifndef POT_H
#define POT_H

#include "cmsis_os2.h" 

// Hacemos visible la ID de la cola para el módulo Principal 
extern osMessageQueueId_t mid_Volumen;

// Función para inicializar el módulo (crear cola y thread)
int Init_POT(void);

#endif