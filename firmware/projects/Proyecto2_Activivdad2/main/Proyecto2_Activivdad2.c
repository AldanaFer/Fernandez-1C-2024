/*! @mainpage Proyecto2_Activivdad2
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section Actividad 2 del Proyecto 2
 *
 * @section genDesc Descripcion General
 * 
 * Utilizazr interrupciones para medir la distancia con el modulo HcSr04 y mostrar la 
 * informacion con los LEDS y con un display, pudiendo modificar si activamos o detenemos la 
 * medicion, o si la mostramos en el display.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/04/2024 | Document creation		                         |
 * | 19/04/2024 | Document completion		                     |
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "led.h"
#include "switch.h"
#include "lcditse0803.h"
#include "hc_sr04.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
/*==================[macros and definitions]=================================*/
#define PERIODO_MOSTRAR_US 150000
#define PERIODO_MEDIR_US 500000
/*==================[internal data definition]===============================*/
uint16_t distancia = 0; 
bool mostrar = true;
bool medir = true;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

/**
 * @fn void activarDetenerMedicion()
 * @brief Función que cambia el valor del bool medir cuando se la llama.
 */
void activarDetenerMedicion()
{
	medir = !medir;
};

/**
 * @fn void HOLD()
 * @brief Función que cambia el valor del bool mostrar cuando se la llama.
 */
void HOLD()
{
	mostrar = !mostrar;
};

/**
 * @brief Función invocada en la interrupción del timer A, se lo utiliza para medir la distancia.
 */
void FuncTimerMedir(void* param)
{
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);   
}

/**
 * @brief Función invocada en la interrupción del timer B, se lo utiliza para mostrar la distancia.
 */
void FuncTimerMostrar(void* param)
{
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);  
}

/**
 * @brief Tarea encargada de medir la distancia con el HcSr04.
 */
static void taskMedir(void *pvParameter)
{
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		if(medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters(); 
		}
	}
};


/**
 * @brief Tarea encargada de mostrar la distancia, tanto prendiendo y apagando los LEDS segun la distancia 
 * como mostrando el numero de la distancia con el display.
 */
static void taskMostrar(void *pvParameter)
{
    while(true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
		if(distancia < 10) 
		{
			LedOff(LED_1); 
			LedOff(LED_2); 
			LedOff(LED_3); 
		} 
		else 
			if( distancia < 20 )
			{
				LedOn(LED_1); 
				LedOff(LED_2); 
				LedOff(LED_3);
			}
			else 
				if(distancia < 30)
				{
					LedOn(LED_1); 
					LedOn(LED_2); 
					LedOff(LED_3);
				}
				else 
				{
					LedOn(LED_1); 
					LedOn(LED_2); 
					LedOn(LED_3);	
				}
		if(mostrar)
		{
			LcdItsE0803Write(distancia); 
		}
	}
};

/*==================[external functions definition]==========================*/

/*==================[main]==========================*/ 
void app_main(void)
{
	LcdItsE0803Init();
	HcSr04Init(GPIO_3, GPIO_2); 
	SwitchesInit();
	LedsInit();

	SwitchActivInt(SWITCH_1, activarDetenerMedicion, NULL);
	SwitchActivInt(SWITCH_2, HOLD, NULL);

    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = PERIODO_MEDIR_US,
        .func_p = FuncTimerMedir,
        .param_p = NULL
    };
    TimerInit(&timer_medir);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = PERIODO_MOSTRAR_US,
        .func_p = FuncTimerMostrar,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

	xTaskCreate(&taskMedir, "Medir", 512, NULL, 5, &medir_task_handle);
	xTaskCreate(&taskMostrar, "Mostrar", 512, NULL, 5, &mostrar_task_handle);

	TimerStart(timer_medir.timer);
    TimerStart(timer_mostrar.timer);
}
/*==================[end of file]============================================*/
