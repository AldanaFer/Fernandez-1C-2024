/*! @mainpage Proyecto2_Activivdad1
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 5/04/2023 | Document creation		                         |
 * | -/0-/2023 | Document creation		                         |
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
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
#define CONFIG_BLINK_PERIOD_MOSTRAR_US 300000
#define CONFIG_BLINK_PERIOD_MEDIR_US 1000000
/*==================[internal data definition]===============================*/
uint16_t distancia = 0; 
bool mostrar = true;
bool medir = true;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
/*==================[internal functions declaration]=========================*/

void activarDetenerMedicion()
{
	medir = !medir;
};

void HOLD()
{
	mostrar = !mostrar;
};
/**
 * @brief Función invocada en la interrupción del timer A
 */
void FuncTimerMedir(void* param)
{
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_1 */
}

/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerMostrar(void* param)
{
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);    /* Envía una notificación a la tarea asociada al LED_2 */
}


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

    /* Inicialización de timers */
    timer_config_t timer_medir = {
        .timer = TIMER_A,
        .period = CONFIG_BLINK_PERIOD_MEDIR_US,
        .func_p = FuncTimerMedir,
        .param_p = NULL
    };
    TimerInit(&timer_medir);

    timer_config_t timer_mostrar = {
        .timer = TIMER_B,
        .period = CONFIG_BLINK_PERIOD_MOSTRAR_US,
        .func_p = FuncTimerMostrar,
        .param_p = NULL
    };
    TimerInit(&timer_mostrar);

	xTaskCreate(&taskMedir, "Medir", 512, NULL, 5, NULL);
	xTaskCreate(&taskMostrar, "Mostrar", 512, NULL, 5, NULL);

	TimerStart(timer_medir.timer);
    TimerStart(timer_mostrar.timer);
}
/*==================[end of file]============================================*/
