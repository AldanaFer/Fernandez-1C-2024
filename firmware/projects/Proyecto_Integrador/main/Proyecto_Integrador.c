/*! @mainpage Proyecto_Integrador
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "iir_filter.h"

/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_ANALOGICO_US 	2000
#define PERIODO_MEDIR 				1000    // lo que dura la contraccion dividido 16
#define FRECUENCIA_PASA_ALTO 		0.1
#define FRECIENCIA_PASA_BAJO		5
#define SIZE_EMG 					16
#define CHUNK               		16
/*==================[internal data definition]===============================*/
TaskHandle_t Leer_Procesar_Analogico_task_handle = NULL; 
TaskHandle_t contraccion_task_handle = NULL;
uint16_t lectura = 0;
uint16_t umbral_0 = 1;
float EMG [SIZE_EMG];
float EMGfiltrado [SIZE_EMG];
bool control; 
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función invocada en la interrupción del timer A, para convertir a analogico
 */
void FuncTimerConvertirAnalogico(void* param)
{
    vTaskNotifyGiveFromISR(Leer_Procesar_Analogico_task_handle, pdFALSE);    
}

// ********************A N T E S***********************
/**
 * @brief Tarea encargada de leer el dato analogico y mandarlo como string 
 */


void procesarEMG()
{
	for(uint8_t j = 0; j < SIZE_EMG; j++)
	{
		//HiPassFilter(&EMG[j], EMGfiltrado, CHUNK);
		//HiPassFilter(EMG, EMGfiltrado, CHUNK);
		//LowPassFilter(EMGfiltrado, EMGfiltrado, CHUNK);

		j+=CHUNK;
	}
}
/*
		HiPassFilter(&ecg[indice], ecg_filt, CHUNK);
		LowPassFilter(ecg_filt, ecg_filt, CHUNK);

*/

void mostrarEMG()
{
	for(int j = 0; j<SIZE_EMG; j++)
	{
		UartSendString(UART_PC, (const char* )  UartItoa(EMGfiltrado[j], 10));
		UartSendString(UART_PC, "\r"); 
	}
};



/**
 * @brief Tarea encargada de leer el dato analogico y guardarlo en la variable general lectura 
 */

static void task_Leer_Procesar_Analogico(void *pvParameter)
{
	uint8_t contador = 0; 
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		
		AnalogInputReadSingle(CH1, &lectura);
		EMG[contador] = (float)lectura; 

		 if(contador == 15)
		 {
		 	//procesarEMG();
			mostrarEMG();
			contador = 0; 
		}
	}
};


/**
 * @brief Tarea encargada de leer el dato analogico y mandarlo como string 
 */

// static void taskContraccion(void *pvParameter)
// {
// 	while (true)
// 	{
// 		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 

// 		//// En vez de hacerlo asi, leer el emg en la de analogica 
// 		// o guardar todo (200 valores) y el primero que es mayor 
// 		//if(control)
// 		//{
// 			for (uint8_t i = 0; i < size_EMG; i++)
// 			{
// 				AnalogInputReadSingle(CH1, &lectura);
// 				EMG[i] = float(lectura); 
// 				vTaskDelay(PERIODO_MEDIR / portTICK_PERIOD_MS);   // lo que dura la contraccion sobre 16
// 			}

// 		//}

// 		// procesar la señal 
		

// 	}
// };


/*==================[external functions definition]==========================*/

void app_main(void)
{


	// ********************D E S P U E S***********************

	analog_input_config_t analogico_i =	{	
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

    HiPassInit(FRECUENCIA_PASA_ALTO, 1.0, ORDER_2); 
	//LowPassInit(FRECIENCIA_PASA_BAJO, 30, ORDER_2);


	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 115200,		
		.func_p = NULL,			
		.param_p = NULL
	}; 

	UartInit(&miUart);
	AnalogOutputInit();

	xTaskCreate(&task_Leer_Procesar_Analogico, "Leer y Procesar Analogico", 4096, NULL, 5, &Leer_Procesar_Analogico_task_handle);
	TimerStart(timerAnalogico.timer);






};
/*==================[end of file]============================================*/