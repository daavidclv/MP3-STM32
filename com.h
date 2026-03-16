#ifndef COM_H
#define COM_H

#include <stdint.h>

// -- VARIABLES DE TIEMPO 
extern uint8_t horas;
extern uint8_t minutos;
extern uint8_t segundos;

// --- FUNCIONES PÚBLICAS ---
void UART_Init(void);
void UART_MSG(char *mensaje); // Usaremos esta función para enviar

#endif 