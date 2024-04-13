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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD_MOSTRAR 300
#define CONFIG_BLINK_PERIOD_MEDIR 1000
#define CONFIG_BLINK_PERIOD_SWITCH 40
/*==================[internal data definition]===============================*/
uint16_t distancia = 0; 
bool mostrar = true;
bool medir = true;
/*==================[internal functions declaration]=========================*/
static void taskSwitch(void *pvParameter)
{
	uint8_t teclas;
	while (true)
	{
		teclas = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
				medir = !medir;
    		break;
    		case SWITCH_2:
				mostrar = !mostrar;
    		break;
    	}
		vTaskDelay(CONFIG_BLINK_PERIOD_SWITCH / portTICK_PERIOD_MS);
	}
	
};
static void taskMedir(void *pvParameter)
{
	while (true)
	{
		if(medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters(); 
		}
		vTaskDelay(CONFIG_BLINK_PERIOD_MEDIR / portTICK_PERIOD_MS);
	}
};
static void taskMostrar(void *pvParameter)
{
    while(true)
	{
		if(distancia< 10) 
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
		vTaskDelay(CONFIG_BLINK_PERIOD_MOSTRAR / portTICK_PERIOD_MS);
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

	xTaskCreate(&taskMedir, "Medir", 512, NULL, 5, NULL);
	xTaskCreate(&taskSwitch, "Switch", 512, NULL, 5, NULL);
	xTaskCreate(&taskMostrar, "Mostrar", 512, NULL, 5, NULL);
}
/*==================[end of file]============================================*/
