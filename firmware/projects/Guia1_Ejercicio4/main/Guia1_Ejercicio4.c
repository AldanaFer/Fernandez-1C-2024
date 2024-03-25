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
#include <gpio_mcu.h>
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
typedef struct 
{
	gpio_t pin;			/*!< GPIO pin number */
	io_t dir;			/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
{
	uint8_t auxiliar; 
	for(uint8_t i =0; i<digits; i++)
	{
		// 0 a digitis    posicion 0 es el menos significativo
		auxiliar = data % 10; 
		data /= 10; 
		bcd_number[i] = auxiliar;
	}
	return 0; 
}; 

int8_t  cambiarEstadoGPIO (uint8_t bcd,  gpioConf_t * puertosGPIO)
{
	uint8_t mascara = 1; 
	for(uint8_t i = 0; i<4; i++)
	{
		if(bcd & mascara )
		{
			GPIOOn(puertosGPIO[i].pin);  
		}
		else  
		{
			GPIOOff(puertosGPIO[i].pin);
		}
		mascara = mascara<<i;
	}
	return 0;
};
//como inicializar los gpio



void app_main(void)
{
	//Ejercicio 4
uint32_t valor = 59832; 
uint8_t cantidadDigitos = 5; 
uint8_t bcdArreglo[10];
convertToBcdArray(valor, cantidadDigitos, bcdArreglo); 
for(uint8_t i = cantidadDigitos  ; i > 0 ; i--)
	{	
		printf("El numero es: %u", bcdArreglo[(i-1)]); 
		printf("\t");

		
	}

// Ejercicio 5
uint8_t bcd = 8; 


gpioConf_t vector[4] = 
{
	{GPIO_20, GPIO_OUTPUT},
	{GPIO_21, GPIO_OUTPUT},
	{GPIO_22, GPIO_OUTPUT},
	{GPIO_23, GPIO_OUTPUT}
}; 


for(uint8_t i = 0; i<4; i++)
	{
		GPIOInit(vector[i].pin, vector[i].dir); 
	}


cambiarEstadoGPIO(bcd, vector); 





}

/*==================[end of file]============================================*/