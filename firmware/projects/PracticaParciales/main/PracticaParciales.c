/*! @mainpage Practica Parciales
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section PracticaParciales
 *
 * @section Diseñar e implementar el firmware de un sistema de adquisición que cumpla con los
siguientes requerimientos:

a. Digitalizar una señal de un sensor de temperatura conectado al canal CH1 de
la EDU-CIAA a una frecuencia de muestreo de 20Hz. Modificar el driver que
considere necesario para la temporización y emplear interrupciones: Timer y
ADC.
b. Calcular el valor máximo, el valor mínimo y el promedio del último segundo
de señal.
c. Seleccionar mediante tres teclas de la PC, a través de la comunicación serie,
el parámetro a presentar en un display LCD (máximo, mínimo o promedio). El
mismo valor se debe enviar a través de la UART a la PC (en ascii). Emplear
la interrupción de recepción de datos de la UART.
d. El sistema deberá informar mediante la activación de los led el valor
seleccionado a presentar en el LCD (rojo: máximo, amarillo: mínimo, verde:
promedio).
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 * | 10/06/2024 | Document completion		                     |
 *
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "uart_mcu.h"
//#include "led.h"
#include "ejemplo.h"  // es el drive led pero con otro nombre
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_DIGITAL_US 50000
#define CANTIDAD_MUESTRAS 20
/*==================[internal data definition]===============================*/
TaskHandle_t convertir_digital_task_handle = NULL;
static float temperaturas [CANTIDAD_MUESTRAS] = { 0 }; // 20 mediciones es 1 segundo con esa frecuencia
static uint8_t contador = 0;     
static float promedio, minimo, maximo;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función invocada en la interrupción del timer A, para convertir a analogico
 */
void FuncTimerConvertirDigital(void* param)
{
    vTaskNotifyGiveFromISR(convertir_digital_task_handle, pdFALSE);    
}

/**
 * @brief Tarea encargada de leer el dato analogico y mandarlo como string 
 */
void procesar()
{
	promedio = 0; 
	minimo = temperaturas[0]; 
	maximo = temperaturas[0]; 
	for (uint8_t i = 0; i < CANTIDAD_MUESTRAS; i++)
	{
		if(temperaturas[i] < minimo)
		{
			minimo = temperaturas[i]; 
		}
		if(temperaturas[i] < maximo)
		{
			maximo = temperaturas[i]; 
		}
		promedio = promedio + temperaturas[i];
	}
	promedio = promedio/CANTIDAD_MUESTRAS; 
	
};

/**
 * @brief Tarea encargada de leer el dato analogico y mandarlo como string 
 */
static void taskDigital(void *pvParameter)
{
	uint16_t lectura;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		AnalogInputReadSingle(CH1, &lectura);
		temperaturas[contador] = (float) lectura; 
		contador ++; 
		if(! (contador < CANTIDAD_MUESTRAS) )
		{
			procesar(); 
			contador = 0; 
		}
	}
};


/**
 * @brief Función invocada por el UART cuando se recibe un dato. a para minimo, b para maximo y c para promedio
 */
void funcionUart()
{
	//rojo: máximo, amarillo: mínimo, verde: promedio
	uint32_t mostrar = 0;
	uint8_t letra; 
	UartReadByte(UART_PC, &letra); 
	switch (letra)
	{
	case 'a':
		mostrar = (uint32_t) minimo;
		LedOn(LED_2); 
		LedOff(LED_1);
		LedOff(LED_3);
		break;
	case 'b': 
		mostrar = (uint32_t) maximo; 
		LedOn(LED_3); 
		LedOff(LED_2);
		LedOff(LED_1);
		break;
	case 'c': 
		mostrar = (uint32_t) promedio; 
		LedOn(LED_1); 
		LedOff(LED_2);
		LedOff(LED_3);
		break;
	}
	UartSendString(UART_PC, (const char* )UartItoa(mostrar, 10)  ); 
	UartSendString(UART_PC, "\r\n"); 
};

/*==================[external functions definition]==========================*/
void app_main(void)
{
	analog_input_config_t analogico_i =
	{
		.input = CH1, 
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL
	};
	AnalogInputInit(&analogico_i);

    timer_config_t timerAnalogico = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_DIGITAL_US,
        .func_p = FuncTimerConvertirDigital,
        .param_p = NULL
    };
	TimerInit(&timerAnalogico);

	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 115200,		
		.func_p = funcionUart,			
		.param_p = NULL
	}; 
	UartInit(&miUart);

	AnalogOutputInit();

	xTaskCreate(&taskDigital, "Digital", 4096, NULL, 5, &convertir_digital_task_handle);
	TimerStart(timerAnalogico.timer); 

	LedsInit();
}
/*==================[end of file]============================================*/