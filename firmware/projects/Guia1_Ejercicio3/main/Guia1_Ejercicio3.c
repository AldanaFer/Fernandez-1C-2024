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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
void funcionLed ( struct leds *led )
{
	if(led->mode())
	{

	}
}; 



/*==================[external functions definition]==========================*/
enum modoLed
{
	ON = 1,
	OFF = 2,
	TOGGLE = 3
}; 


struct leds
{
	uint8_t n_led;      //  indica el número de led a controlar
	uint8_t n_ciclos;  // indica la cantidad de ciclos de encendido/apagado
	uint8_t periodo;   // indica el tiempo de cada ciclo
	modoLed mode;      // ON, OFF, TOGGLE
} my_leds;



void app_main(void){
	printf("Hello world!\n");









}



/*==================[end of file]============================================*/