# 🎵 Reproductor MP3 sobre STM32 (NUCLEO-F429ZI)

Reproductor de audio funcional implementado sobre un microcontrolador STM32, con manejo de interrupciones, comunicación SPI/I2C y UART.

## 📋 Descripción

Firmware desarrollado en C para la placa **NUCLEO-F429ZI** (STM32F429) que implementa un reproductor MP3 con interfaz de usuario física: control mediante joystick, potenciómetro, pantalla LCD y LED RGB de estado. El proyecto integra varios periféricos del microcontrolador trabajando de forma concurrente mediante interrupciones y threads.

## 🛠️ Stack técnico

- **Microcontrolador:** STM32F429ZI (Nucleo-144)
- **Lenguaje:** C
- **IDE:** Keil µVision
- **Periféricos:** ADC, PWM, SPI/I2C, UART, GPIO, LCD, joystick

## 📁 Módulos principales

| Archivo | Función |
|---|---|
| `main.c` | Punto de entrada y lógica principal |
| `mp3.c` / `mp3.h` | Decodificación y control de reproducción |
| `lcd.c` | Control de la pantalla |
| `joystick.c` | Lectura de entrada de usuario |
| `pot.c` | Lectura de potenciómetro (volumen) |
| `adc.c` | Conversión analógico-digital |
| `com.c` | Comunicación (UART/SPI/I2C) |
| `pwm.c` | Generación de PWM |
| `rgb.c` | Control del LED de estado |
| `Thread_Temp.c` | Gestión de hilos/temporización |
| `clock.c` | Configuración de reloj del sistema |

## ⚙️ Cómo compilarlo

1. Abrir `MP3.uvprojx` en Keil µVision
2. Compilar y flashear sobre una NUCLEO-F429ZI

## 📄 Documentación adicional

El repo incluye datasheet y manual de referencia del STM32F429 y la documentación técnica del proyecto (`B3_Documentacion.docx.pdf`) con más detalle de la implementación.

## 🎯 Lo más complejo del proyecto

- Sincronización de múltiples periféricos (audio, pantalla, entrada de usuario) sin bloquear el sistema
- Gestión de interrupciones para mantener la reproducción fluida mientras se atienden otras tareas

## 👤 Autor

David Calvo Heredero — [LinkedIn](https://www.linkedin.com/in/david-calvo-heredero-20b20a33a) · [GitHub](https://github.com/daavidclv)
