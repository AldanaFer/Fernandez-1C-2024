/*! @mainpage Proyecto2_Activivdad1
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section Actividad 1 del Proyecto 2
 *
 * @section genDesc Descripcion General
 * 
 *  Medir la distancia con el modulo HcSr04 y mostrar la informacion con los LEDS 
 *  y un display, pudiendo modificar si activamos o detenemos la medicion, o si 
 *  la mostramos en el display.
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 05/04/2024 | Document creation		                         |
 * | 12/04/2024 | Document completion		                     |
 *
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
#define PERIODO_MOSTRAR 300
#define PERIODO_MEDIR 1000
#define PERIODO_SWITCH 40
/*==================[internal data definition]===============================*/
uint16_t distancia = 0; 
bool mostrar = true;
bool medir = true;
/*==================[internal functions declaration]=========================*/

/**
 * @brief Tarea encargada del manejo de los switch, cambiando el valor de los bool medir y mostrar.
 */
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
		vTaskDelay(PERIODO_SWITCH / portTICK_PERIOD_MS);
	}
};

/**
 * @brief Tarea encargada de medir la distancia desde el HcSr04
 */
static void taskMedir(void *pvParameter)
{
	while (true)
	{
		if(medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters(); 
		}
		vTaskDelay(PERIODO_MEDIR / portTICK_PERIOD_MS);
	}
};


/**
 * 
 * @brief Tarea encargada de mostrar la distancia, tanto prendiendo y apagando los LEDS segun la distancia 
 * como mostrando el numero de la distancia con el display.
 */
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
		vTaskDelay(PERIODO_MOSTRAR / portTICK_PERIOD_MS);
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
