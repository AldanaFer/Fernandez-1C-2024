/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
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
#include <stdlib.h>
#include "led.h"
#include <stdbool.h>
#include "switch.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*==================[macros and definitions]=================================*/
#define periodoRetardo 100
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
enum modoLed
{
	ON ,
	OFF,
	TOGGLE
}; 

struct leds
{
	uint8_t n_led;      
	uint8_t n_ciclos;  
	uint16_t periodo;   
	enum modoLed mode;    
} my_led;


void funcionLed ( struct leds *myLed )
{
	if(myLed->mode == ON)
	{
        if(myLed->n_led==1)
        {
            LedOn(LED_1);
        }
        else
            if(myLed->n_led==2)
            {
            LedOn(LED_2);
            }   
            else
                if(myLed->n_led==3)
                {
                LedOn(LED_3);
                }
            
	}
	else 
	{
    	if(myLed->mode == OFF)
    	{
            if(myLed->n_led==1)   
            {
                LedOff(LED_1);
            }
            else
                if(myLed->n_led==2)
                {
                	LedOff(LED_2);
                }   
                else
                    if(myLed->n_led==3)
                    {
                    	LedOff(LED_3);
                    }
                
    	}
    	else 
    	{
    	    if(myLed->mode == TOGGLE)
    	    {
    	        uint8_t auxiliar = 0; 
    	        while(auxiliar < myLed->n_ciclos)
    	        {
    	            if(myLed->n_led==1)
    	            {
    	                LedToggle(LED_1);
    	            }
    	            else 
    	            {
    	                if(myLed->n_led==2)
    	                {
    	                    LedToggle(LED_2);
    	                }
    	                else 
    	                {
    	                    if(myLed->n_led==3)
    	                    {
    	                        LedToggle(LED_3);
    	                    }
    	                }
    	            }
    	            auxiliar++;
					for(uint8_t aux=0; aux < (myLed->periodo/periodoRetardo); aux++)
					{
						vTaskDelay(periodoRetardo / portTICK_PERIOD_MS);
					}
    	        }
    	    }
    	}
	}
}; 

/*==================[external functions definition]==========================*/

void app_main(void){
 	struct leds led = my_led; 
	LedsInit();

    led.n_led = 1;
    led.mode = TOGGLE;
    led.n_ciclos = 10;
    led.periodo = 400;
    funcionLed(&led);

	led.n_led = 2;
    funcionLed(&led);

	led.n_led = 3;
    funcionLed(&led);

	printf("Fin del programa \n");
}


/*==================[end of file]============================================*/