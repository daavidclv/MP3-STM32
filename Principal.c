#include "cmsis_os2.h"                          // CMSIS RTOS header file
#include "Principal.h"
#include <stdio.h>
#include "joystick.h"
#include "temp.h"
#include "rgb.h"
#include "pwm.h"

/*----------------------------------------------------------------------------
 * Thread 1 'Thread_Name': Sample thread
 *---------------------------------------------------------------------------*/

osThreadId_t tid_Thread;

void Thread_Principal (void *argument);                   // thread function
void Enviar_PC(uint8_t *trama); // UART_MSG()

/*-------COLAS DE COMUNICACION--------*/
osMessageQueueId_t colamensaje;
osMessageQueueId_t leds;
osMessageQueueId_t mid_mp3;
osMessageQueueId_t mid_Volumen;
/*------------------------------------*/


int Init_Thread_Principal (void) {

    /*-----------Colas----------------*/
    colamensaje = osMessageQueueNew(10, sizeof(char), NULL);        //Cola LCD
    leds        = osMessageQueueNew(10, sizeof(uint8_t), NULL);     //Cola LED
    mid_mp3     = osMessageQueueNew(8, 8, NULL);                    //Cola MP3
    colaLCD_L1  = osMessageQueueNew(5,20*sizeof(char),NULL);        //Cola Linea 1
    colaLCD_L2  = osMessageQueueNew(5,20*sizeof(char),NULL);        //Cola Linea 2
    mid_Volumen = osMessageQueueNew(2,sizeof(float),NULL);          //Cola Linea 2

    tid_Thread = osThreadNew(Thread_Principal, NULL, NULL);
    if (tid_Thread == NULL) {
        return(-1);
    }

    return(0);
}

void Thread_Principal (void *argument) {

    /*-----------VARIABLES No Globales----------------*/
    uint8_t gesto = 0;
    uint8_t tipo = 0;
    uint8_t recibido = 1;
    uint8_t recibido_v = 1;
    char buffer_texto[20]; // Buffer temporal para formatear texto (sprintf)
    uint8_t comando_led = 0;
    float voltaje_pot = 0.0f;
    uint8_t vol_actual = 15;
    static uint8_t cuenta_cancion = 0; 
    static volatile uint8_t estado = 0; //0: Reposo, 1:Reproduccion, 2: Programacion
    static float temp_lectura = 0.0f;
    static uint8_t modo_pausa = 1; //0: Reproduciendo , 1:PAusa
    static uint8_t carpeta_actual = 1;
     static volatile uint8_t cuenta = 0;

    //Variables para la lógica de programación
    static uint8_t selector = 0;           // 0: Horas, 1: Minutos, 2: Segundos
    static uint8_t aux_h, aux_m, aux_s;   // Variables temporales para editar
    static uint8_t entrada_prog = 1;     // Bandera para inicializar valores al entrar

    //VARIABLES PARA EL MENU
    const char *opciones_menu[] = {
        "1. Reproductor",
        "2. Config Hora",
        "3. Monitor T/Temp"
    };

    // Estado del Reproductor/variables numericas
    static uint8_t cancion_actual = 1;
    static uint8_t t_min = 0;
    static uint8_t t_seg = 0;
    static uint8_t cero = 0;
    static uint8_t uno = 1;
    static uint8_t dos = 2;
    static uint8_t cuatro = 4;
    static uint32_t tick_anterior = 0;

    // NUEVA VARIABLE: Modo Continuo (0: Manual, 1: Auto)
    static uint8_t modo_continuo = 0;
    //MENU
    static int8_t menu = 0; // Índice de la opción seleccionada

    // Variable para controlar el log único
    static uint8_t ultimo_estado = 255;
    
    //Variable de modo automatico
    char *modo_auto;

//--------------------------------------------

    /*Con esto inicamos la tarjeta, pero no se si ponerlo aqui o hacerlo una vez se entre en el modo reproduccion*/
    osMessageQueuePut(mid_mp3, iniciar, 0, osWaitForever);
    Enviar_PC(iniciar);


    while (1) {

        osMessageQueueGet(colatemp, &temp_lectura, NULL,0U);

        // CONTROL DE EVENTOS MP3 (Tarjeta y FIN DE CANCIÓN)

        if (osMessageQueueGet(mid_mp3_rx, &recibido, NULL, 0U) == osOK) {
            
            // FIN DE CANCIÓN (0x3D recibido en mp3.c) ---
            if (recibido == 3) {
                // Reiniciamos contador siempre que acabe una canción
                t_min = 0;
                t_seg = 0;

                if (estado == 1 && modo_continuo == 1) {
                    // Si estamos en modo continuo, pasamos a la siguiente
                    cancion_actual++;
                    osMessageQueuePut(mid_mp3, cmd_next, 0, 0);
                    Enviar_PC(cmd_next);
                    UART_MSG("AUTO: Cancion terminada -> Siguiente\r\n");
                } else {
                    UART_MSG("INFO: Cancion terminada (Stop)\r\n");
                }
            }
            // --- CÓDIGO 2: TARJETA FUERA ---
            else if (recibido == 2) {
                estado = 0;
                osMessageQueuePut(leds, &cero, 0, 0U);
                osMessageQueuePut(mid_PWM, &dos, 0, 0U);
                UART_MSG("ALERTA: Tarjeta fuera.\r\n");
            }
            // --- CÓDIGO 1: TARJETA DENTRO ---
            else if (recibido == 1) {
                osMessageQueuePut(leds, &cuatro, 0, 0U);
                UART_MSG("INFO: Tarjeta dentro.\r\n");
            }
        }

        // Control de Tiempo Canción (SINCRONIZADO) 
        if (estado == 1 && modo_pausa == 0) {
            // Solo entra aquí si ha pasado 1 segundo real (1000 ticks)
            if (osKernelGetTickCount() - tick_anterior >= 1000) {
                tick_anterior += 1000; // Avanzamos la marca de tiempo

                t_seg++;
                if (t_seg >= 60) {
                    t_seg = 0;
                    t_min++;
                }
            }
        } else {
            // Si pausamos o cambiamos de estado, actualizamos el tick para que no de saltos
            tick_anterior = osKernelGetTickCount();
        }
        // -------------------------------------------------------------

        // INICIO CONTROL DE VOLUMEN
        if (osMessageQueueGet(mid_Volumen, &voltaje_pot, NULL, 0U) == osOK) {
            float calculo = ((voltaje_pot / 3.25f) * 30.0f) + 0.5f; //para que de 0-30

            // Limites obligatorios por si el cálculo da 31 o 32
            if (calculo > 30.0f) {
                calculo = 30.0f;
            }
            if (calculo < 0.0f) {
                calculo = 0.0f;
            }

            uint8_t nuevo_volumen = (uint8_t)calculo;

            if (nuevo_volumen != vol_actual) {
                vol_actual = nuevo_volumen;

                trama_vol[3] = 0x06;
                trama_vol[6] = vol_actual;

                osMessageQueuePut(mid_mp3, trama_vol, 0, 0);
                Enviar_PC(trama_vol);
            }
        }
        // ------------------ FIN CONTROL DE VOLUMEN ------------------

        switch(estado) {
            case 0: //MENU
                if (estado != ultimo_estado) {
                    UART_MSG("ESTADO: MENU\r\n");
                    ultimo_estado = estado;
                }

                if (recibido == 2) {
                    osMessageQueuePut(leds, &cero, 0, 0);
                } else {
                    // Título
                    osMessageQueuePut(leds, &cuatro, 0, 0);
                    osMessageQueuePut(colaLCD_L1, "--- MENU SBM ---", 0, 0);
                    osMessageQueuePut(mid_mp3, cmd_sleep, 0, osWaitForever);//Dormimos al MP3
                    // Opción con flecha ">"
                    snprintf(buffer_texto, 20, "> %s", opciones_menu[menu]);
                    osMessageQueuePut(colaLCD_L2, buffer_texto, 0, 0);

                    comando_led = 3;
                    osMessageQueuePut(leds, &comando_led, 0, 0);

                    // 2. GESTIÓN JOYSTICK
                    if (osMessageQueueGet(colaJoystickgesto, &gesto, NULL, 0U) == osOK) {
                        if (osMessageQueueGet(colaJoystickpulsacion, &tipo, NULL, osWaitForever) == osOK) {
                            if(gesto == 4 && tipo == 0) { // ARRIBA -> Siguiente opción
                                if(menu < 2) {
                                    menu++;
                                }
                                osMessageQueuePut(mid_PWM, &uno, 0, 0);
                                UART_MSG("JOYSTICK: ABAJO - Siguiente opcion\r\n");
                            } else if(gesto == 1 && tipo == 0) { // ABAJO -> Opción anterior
                                if(menu > 0) {
                                    menu--;
                                }
                                osMessageQueuePut(mid_PWM, &uno, 0,0);
                                UART_MSG("JOYSTICK: ARRIBA - Opcion anterior\r\n");
                            } else if(gesto == 16 && tipo == 0) { // CENTRO CORTA -> ENTRAR
                                if (menu == 0) {
                                    estado = 1;
                                    osMessageQueuePut(leds,&uno,0,0U);//Reproductor
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                } else if (menu == 1) {
                                    estado = 2; //Config Hora
                                } else if (menu == 2) {
                                    estado = 3; //Reposo(MONITOR/TEMP)
                                }
                              
                                entrada_prog = 1;
                                osMessageQueuePut(mid_PWM, &uno, 0, 0); // Bip confirmación
                                UART_MSG("JOYSTICK: CENTRO - Entrar opcion\r\n");
                            }
                        }
                    }
                }
                break;

            case 1:/*REPRODUCCION*/
                if (estado != ultimo_estado) {
                    UART_MSG("ESTADO: REPRODUCCION\r\n");
                    ultimo_estado = estado;
                }

                if (recibido == 2) {
                    estado = 0;
                } else {
                    // Si modo_continuo == 1 A ------- modocoontinuo ==0 M(manual)"
                    if(modo_continuo==1){
                      modo_auto = "A";
                    }else{
                      modo_auto="M";
                    }
                   // osMessageQueuePut(mid_mp3, cmd_resume, 0, osWaitForever);         
                    osMessageQueuePut(mid_mp3, cmd_awake, 0, osWaitForever);//Despertamos al MP3
                    snprintf(buffer_texto, 20, "%s F:%02d C:%02d    V:%02d", modo_auto, carpeta_actual, cuenta_cancion, vol_actual);
                    osMessageQueuePut(colaLCD_L1, buffer_texto, 0, 0);
                    //Seguynda linea
                    snprintf(buffer_texto, 20, "     T: %02d:%02d", t_min, t_seg);
                    osMessageQueuePut(colaLCD_L2, buffer_texto, 0, 0);

                    //JOYSTICK
                    if (osMessageQueueGet(colaJoystickgesto, &gesto, NULL,0U) == osOK) { /*Con esto manejaremos las interrupciones para el joy*/
                        if (osMessageQueueGet(colaJoystickpulsacion, &tipo, NULL,0U) == osOK) {

                            if( gesto == 16 && tipo == 0) { /*Pausa/play(CENTRO)*/
                                if(modo_pausa == 0) {
                                    osMessageQueuePut(mid_mp3, cmd_pause, 0, osWaitForever);
                                    modo_pausa = 1;
                                    Enviar_PC(cmd_pause);
                                    osMessageQueuePut(leds,&dos,0,0U);
                                    UART_MSG("Pausado\r\n");
                                } else if(modo_pausa == 1) {
                                    osMessageQueuePut(mid_mp3, cmd_resume, 0, osWaitForever);
                                    modo_pausa = 0;
                                    Enviar_PC(cmd_resume);
                                    osMessageQueuePut(leds,&uno,0,0U);
                                    UART_MSG("MUSIQUETE ON\r\n");
                                }
                                osMessageQueuePut(mid_PWM, &uno, 0,0); //PITIDO

                            } else if(gesto == 1 && tipo == 0) { /*Siguiente carpeta (ARRIBA)*/
                                cuenta++;
                                modo_pausa = 0;                                    
                                osMessageQueuePut(leds,&uno,0,0U);
                                cuenta_cancion = 0;
                                if (cuenta>2) {
                                    cuenta = 0;
                                }
                                if (cuenta == 0) {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 1;
                                    Enviar_PC(trama_c);
                                } else if(cuenta == 1) {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 2;
                                    Enviar_PC(trama_c);
                                } else {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x03, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 3;
                                    Enviar_PC(trama_c);
                                }
                                cancion_actual = 1;
                                t_min = 0;
                                t_seg = 0; // RESET TIEMPO
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("Carpeta cambiada\r\n");

                            } else if(gesto == 8 && tipo == 0) { /*Anterior canción (IZQUIERDA)*/
                                modo_pausa = 0;                                    
                                osMessageQueuePut(leds,&uno,0,0U);
                                if(cancion_actual>1) {
                                    cancion_actual--;
                                } else {
                                    cancion_actual=1;
                                }

                                Enviar_PC(cmd_prev);
                                t_min = 0;
                                t_seg = 0; //RESET TIEMPO
                                UART_MSG("Cancion anterior\r\n");
                                if (carpeta_actual == 1){
                                  cuenta_cancion--;
                                    if  (cuenta_cancion > 10) {
                                      cuenta_cancion = 1;
                                      uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x02, 0xEF};
                                      osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }else{
                                      osMessageQueuePut(mid_mp3, cmd_prev, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }
                                }else if(carpeta_actual == 2){
                                  cuenta_cancion--;
                                    if(cuenta_cancion > 10) {
                                        cuenta_cancion = 2;
                                        uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x03, 0xEF};
                                        osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                        osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }else{
                                      osMessageQueuePut(mid_mp3, cmd_prev, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }
                                }else if(carpeta_actual == 3){
                                  uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x03, 0x01, 0xEF};
                                  osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                  osMessageQueuePut(mid_PWM, &uno,0,0);
                                }
                            } else if(gesto == 2 && tipo == 0) { /*Siguiente canción(DERECHA)*/
                                modo_pausa = 0;                                    
                                osMessageQueuePut(leds,&uno,0,0U);
                                t_min = 0;
                                t_seg = 0; // RESET TIEMPO
                                UART_MSG("Siguiente cancion\r\n");
                                if (carpeta_actual == 1){
                                    cuenta_cancion++;
                                    if  (cuenta_cancion == 2) {
                                    cuenta_cancion = 0;
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }else{
                                      osMessageQueuePut(mid_mp3, cmd_next, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                  Enviar_PC(cmd_next);}
                                }else if(carpeta_actual == 2){
                                  cuenta_cancion++;
                                    if(cuenta_cancion == 3) {
                                      cuenta_cancion = 0;
                                      uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x01, 0xEF};
                                      osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                    }else{
                                      osMessageQueuePut(mid_mp3, cmd_next, 0, osWaitForever);
                                      osMessageQueuePut(mid_PWM, &uno,0,0);
                                      Enviar_PC(cmd_next);}
                                }else if(carpeta_actual == 3){
                                  uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x03, 0x01, 0xEF};
                                  osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                  osMessageQueuePut(mid_PWM, &uno,0,0);
                                }

                            //DERECHA LARGA -> MODO CONTINUO 
                            } else if(gesto == 2 && tipo == 1) {
                                
                                modo_continuo = 1;
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("MODO CONTINUO: ACTIVADO\r\n");

                            //IZQUIERDA LARGA -> SALIR MODO CONTINUO
                            } else if(gesto == 8 && tipo == 1) {
                                modo_continuo = 0;
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("MODO CONTINUO: DESACTIVADO\r\n");

                            } else if(gesto == 4 && tipo == 0) { /*Carpeta anterior(ABAJO)*/
                                cuenta--;
                                cuenta_cancion = 0;
                                modo_pausa = 0;                                    
                                osMessageQueuePut(leds,&uno,0,0U);
                                if (cuenta>10) {
                                    cuenta = 2;
                                }
                                if (cuenta == 0) {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x01, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 1;
                                    Enviar_PC(trama_c);
                                } else if(cuenta == 1) {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x02, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 2;
                                    Enviar_PC(trama_c);
                                } else {
                                    uint8_t trama_c[8] = {0x7E, 0xFF, 0x06, 0x0F, 0x00, 0x03, 0x01, 0xEF};
                                    osMessageQueuePut(mid_mp3, &trama_c, 0, osWaitForever);
                                    carpeta_actual = 3;
                                    Enviar_PC(trama_c);
                                }
                                cancion_actual = 1;
                                t_min = 0;
                                t_seg = 0; // RESET TIEMPO
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("Carpeta anterior\r\n");

                            } else if( gesto == 16 && tipo == 1) { /*Siguiente estado(CENTRO)*/
                                estado=0;
                                osMessageQueuePut(mid_mp3, cmd_pause, 0, 0);
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                Enviar_PC(cmd_pause);
                                UART_MSG("JOYSTICK: CENTRO LARGO - Salir\r\n");
                            }
                        }
                    }
                }
                break;

            case 2:/*PROGRAMACION DE HORA*/
                if (estado != ultimo_estado) {
                    UART_MSG("ESTADO: CONFIG HORA\r\n");
                    ultimo_estado = estado;
                }

                if (recibido == 2) {
                    estado=0;
                } else {
                    // Inicialización de variables temporales al entrar
                    osMessageQueuePut(mid_mp3, cmd_sleep, 0, osWaitForever);//Dormimos al MP3
                    if (entrada_prog) {
                        aux_h = horas;
                        aux_m = minutos;
                        aux_s = segundos;
                        selector = 0;       //Empezamos en las HORAS
                        entrada_prog = 0;
                    }

                    osMessageQueuePut(colaLCD_L1, "CONFIG HORA", 0, 0);

                    // selector -> 0:Horas, 1:Minutos, 2:Segundos
                    if (selector == 0) {
                        snprintf(buffer_texto, 20, "[%02d]:%02d:%02d", aux_h, aux_m, aux_s);
                    } else if (selector == 1) {
                        snprintf(buffer_texto, 20, "%02d:[%02d]:%02d", aux_h, aux_m, aux_s);
                    } else {
                        snprintf(buffer_texto, 20, "%02d:%02d:[%02d]", aux_h, aux_m, aux_s);
                    }
                    osMessageQueuePut(colaLCD_L2, buffer_texto, 0, 0);

                    //LED Rojo Parpadeando (Aviso de modo configuración)

                    // JOYSTICK
                    if (osMessageQueueGet(colaJoystickgesto, &gesto, NULL, 0U) == osOK) {
                        if (osMessageQueueGet(colaJoystickpulsacion, &tipo, NULL, osWaitForever) == osOK) {

                            //navegar
                            if(gesto == 2 && tipo == 0) { //DERECHA -> Ir hacia Minutos/Segundos
                                if (selector < 2) {
                                    selector++;
                                }
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("JOYSTICK: DERECHA - Siguiente campo\r\n");
                            } else if(gesto == 8 && tipo == 0) { // IZQUIERDA -> Ir hacia Minutos/Horas
                                if (selector > 0) {
                                    selector--;
                                }
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("JOYSTICK: IZQUIERDA - Campo anterior\r\n");
                            }

                            //Editar
                            else if(gesto == 1 && tipo == 0) { // ARRIBA -> INCREMENTAR
                                if (selector == 0) { // Horas
                                    aux_h++;
                                    if (aux_h > 23) {
                                        aux_h = 0;
                                    }
                                } else if (selector == 1) { // Minutos
                                    aux_m++;
                                    if (aux_m > 59) {
                                        aux_m = 0;
                                    }
                                } else { // Segundos
                                    aux_s++;
                                    if (aux_s > 59) {
                                        aux_s = 0;
                                    }
                                }
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("JOYSTICK: ARRIBA - Incrementar\r\n");

                            } else if(gesto == 4 && tipo == 0) { // ABAJO -> DECREMENTAR
                                if (selector == 0) { // Horas
                                    if (aux_h == 0) {
                                        aux_h = 23;
                                    } else {
                                        aux_h--;
                                    }
                                } else if (selector == 1) { // Minutos
                                    if (aux_m == 0) {
                                        aux_m = 59;
                                    } else {
                                        aux_m--;
                                    }
                                } else { // Segundos
                                    if (aux_s == 0) {
                                        aux_s = 59;
                                    } else {
                                        aux_s--;
                                    }
                                }
                                osMessageQueuePut(mid_PWM, &uno, 0,0);
                                UART_MSG("JOYSTICK: ABAJO - Decrementar\r\n");

                                //CONFIRMACIÓN Y SALIDA 

                            } else if( gesto == 16 && tipo == 0) { //CENTRO CORTA -> GUARDAR CAMBIOS
                                //actualizamo
                                horas = aux_h;
                                minutos = aux_m;
                                segundos = aux_s;
                                osMessageQueuePut(mid_PWM, &uno,0, 0);
                                UART_MSG("JOYSTICK: CENTRO - Guardar cambios\r\n");

                            } else if( gesto == 16 && tipo == 1) { //CENTRO LARGA -> SALIR
                                estado = 0;
                                osMessageQueuePut(mid_PWM, &uno,0, 0);
                                UART_MSG("JOYSTICK: CENTRO LARGO - Salir\r\n");
                            }
                        }
                    }
                }
                break;
            case 3:/*REPOSO*/
                if (estado != ultimo_estado) {
                    UART_MSG("ESTADO: MONITOR/TEMP\r\n");
                    ultimo_estado = estado;
                }

                //Primera linea del LCD
                snprintf(buffer_texto, 20, "SBM 2025  T:%.1fC", temp_lectura);
                osMessageQueuePut(colaLCD_L1, buffer_texto, 0, 0);
                //Segunda Linea
                snprintf(buffer_texto, 20, "%02d:%02d:%02d", horas, minutos, segundos);
                osMessageQueuePut(colaLCD_L2, buffer_texto, 0, 0);

                if (recibido == 1) { // Solo si hay tarjeta
                    if (osMessageQueueGet(colaJoystickgesto, &gesto, NULL,0U) == osOK) {
                        if (osMessageQueueGet(colaJoystickpulsacion, &tipo, NULL,0U) == osOK) {
                            if( gesto == 16 && tipo == 1) {
                                estado=0;
                                osMessageQueuePut(mid_PWM, &uno,0,0);
                                UART_MSG("JOYSTICK: CENTRO LARGO - Salir de reposo\r\n");
                            }
                        }
                    }
                }

                break;
        }
        osDelay(100);
    }
    osThreadYield();                    // suspend thread
}



void Enviar_PC(uint8_t *trama)
{
    char buffer_uart[64];
    sprintf(buffer_uart,"%02d:%02d:%02d---> %02X %02X %02X %02X %02X %02X %02X %02X\r\n", horas, minutos, segundos,
            trama[0],trama[1],trama[2],trama[3],trama[4],
            trama[5],trama[6],trama[7]);

    UART_MSG(buffer_uart);
}