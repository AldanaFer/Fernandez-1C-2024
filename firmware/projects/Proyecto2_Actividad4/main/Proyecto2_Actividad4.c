/*! @mainpage Proyecto2_Actividad4
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section Actividad 4 del Proyecto 2
 *
 * @section genDesc Descripcion General
 * 
 *  Aplicacion que utiliza dos drivers para digitalizar una señal analogica y transmitirla 
 *  por un puerto serie a la PC. Luego se realiza un conversor de digital a analogico para 
 *  poder observar un ECG suministrado por la catedra.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 26/04/2024 | Document creation		                         |
 * | 03/05/2024 | Document completion		                     |
 *
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_ANALOGICO_US 2000
#define CONFIG_PERIOD_DIGITAL_US 4000
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
TaskHandle_t convertir_analog_task_handle = NULL;
TaskHandle_t convertir_digital_task_handle = NULL;
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función invocada en la interrupción del timer A, para convertir a analogico
 */
void FuncTimerConvertirAnalogico(void* param)
{
    vTaskNotifyGiveFromISR(convertir_analog_task_handle, pdFALSE);    
}

/**
 * @brief Función invocada en la interrupción del timer B, para convertir a digital.
 */
void FuncTimerConvertirDigital(void* param)
{
    vTaskNotifyGiveFromISR(convertir_digital_task_handle, pdFALSE);   
}

/**
 * @brief Tarea encargada de leer el dato analogico y mandarlo como string 
 */
static void taskAnalogica(void *pvParameter)
{
	uint16_t dato; 
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		AnalogInputReadSingle(CH1, &dato);
		UartSendString(UART_PC, (const char* )  UartItoa(dato, 10));
		UartSendString(UART_PC, "\r"); 
	}
};

/**
 * @brief Tarea encargada de escribir en el CH0 el dato correspondiende del ECG
 */
static void taskDigital(void *pvParameter)
{
	uint8_t posicion = 0; 
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		AnalogOutputWrite(ecg[posicion]);
		posicion++;
		if( posicion >= BUFFER_SIZE ) 
		{
			posicion = 0;
		}
	}
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
        .period = CONFIG_PERIOD_ANALOGICO_US,
        .func_p = FuncTimerConvertirAnalogico,
        .param_p = NULL
    };
	TimerInit(&timerAnalogico);

	timer_config_t timerDigital = 
	{
   		.timer = TIMER_B,
        .period = CONFIG_PERIOD_DIGITAL_US,
        .func_p = FuncTimerConvertirDigital,
        .param_p = NULL
    };
	TimerInit(&timerDigital);

	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 115200,		
		.func_p = NULL,			
		.param_p = NULL
	}; 
	UartInit(&miUart);

	AnalogOutputInit();

	xTaskCreate(&taskAnalogica, "Analogico", 512, NULL, 5, &convertir_analog_task_handle);
	xTaskCreate(&taskDigital, "Digital", 512, NULL, 5, &convertir_digital_task_handle);
	TimerStart(timerAnalogico.timer); 
	TimerStart(timerDigital.timer); 

}
/*==================[end of file]============================================*/