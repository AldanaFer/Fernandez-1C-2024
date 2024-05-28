/*!
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @brief Programa para mostrar en una pantalla LCD distintos valores. Manteniendo digitos y orden significativo.
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 15/03/2023 | Document creation		                         |
 * | 5/04/2023 | Document completion		                     |
 *
 */
/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <gpio_mcu.h>
#include <esp_system.h>
#include <driver/gpio.h>

/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/**
 * @brief Estructura del Pin Entrada/Salida.
 */
typedef struct
{
	gpio_t pin; /*!< GPIO numero de pin */
	io_t dir;	/*!< GPIO direccion '0' IN (entrada);  '1' OUT (salida)*/
} gpioConf_t;

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
/**
 * @fn int8_t convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief A partir de un valor guarda una cierta cantidad de digitos en un arreglo.
 *
 * @param data Es el valor sin signo de 32 bit.
 * @param digits Es la cantidad de digitos que voy a guardar en el arreglo.
 * @param bcd_number Es un arreglo donde se va a guardar cada digito en cada posicion.
 * @return Retorna 0.
 */
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	for (uint8_t i = 0; i < digits; i++)
	{
		bcd_number[i] = data % 10;
		data /= 10;
	}
	return 0;
};

/**
 * @fn int8_t  cambiarEstadoGPIO (uint8_t bcd,  gpioConf_t * puertos_GPIO)
 * @brief Cambia el estado de cada GPIO, dependiendo los bits de un valor dado.
 *
 * @param bcd Es el valor que indica el estado de los GPIO.
 * @param puertosGPIO Arreglo de los GPIO que van a cambiar de valor.
 * @return Retorna 0.
 */
int8_t cambiarEstadoGPIO(uint8_t bcd, gpioConf_t *puertos_GPIO)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		if (bcd & 1<<i)
		{
			GPIOOn(puertos_GPIO[i].pin);
		}
		else
		{
			GPIOOff(puertos_GPIO[i].pin);
		}
	}
	return 0;
};

/**
 * @fn int8_t mostrarDisplay (uint32_t dato, uint8_t cantidad_Digitos, gpioConf_t * puertos_Numero, gpioConf_t * puertos_control_LCD)
 * @brief Muestra cada digito del valor en su orden correspondiente en LCD, utiliza las funciones convertToBcdArray y cambiarEstadoGPIO.
 *
 * @param dato Es el valor que indica el estado de los GPIO.
 * @param cantDigitos Cantidad de digitos que voy a mostrar.
 * @param puertos_Numero Arreglo de los GPIO que van a formar el numero a mosatrar.
 * @param puertos_control_LCD Arreglo de los GPIO que controla cada LCD.
 * @return Retorna 0.
 */
int8_t mostrarDisplay(uint32_t dato, uint8_t cantidad_Digitos, gpioConf_t *puertos_Numero, gpioConf_t *puertos_control_LCD)
{
	uint8_t bcd_arreglo[cantidad_Digitos];
	convertToBcdArray(dato, cantidad_Digitos, bcd_arreglo);

	for (uint8_t i = 0; i < cantidad_Digitos; i++)
	{
		cambiarEstadoGPIO(bcd_arreglo[i], puertos_Numero);
		GPIOOn(puertos_control_LCD[i].pin);
		GPIOOff(puertos_control_LCD[i].pin);
	}
	return 0;
};

/*==================[main]==========================*/

void app_main(void)
{
	// Ejercicio 4
	uint32_t valor = 123;
	uint8_t cantidadDigitos = 3;
	uint8_t bcdArreglo[10];
	convertToBcdArray(valor, cantidadDigitos, bcdArreglo);

	// Ejercicio 5
	uint8_t bcd = 8;
	gpioConf_t GPIO_20_23[4] =
		{
			{GPIO_20, GPIO_OUTPUT},
			{GPIO_21, GPIO_OUTPUT},
			{GPIO_22, GPIO_OUTPUT},
			{GPIO_23, GPIO_OUTPUT}};
	for (uint8_t i = 0; i < 4; i++)
	{
		GPIOInit(GPIO_20_23[i].pin, GPIO_20_23[i].dir);
	}
	cambiarEstadoGPIO(bcd, GPIO_20_23);

	// Ejercicio 6
	gpioConf_t GPIO_9_18_19[3] =
		{
			{GPIO_9, GPIO_OUTPUT},
			{GPIO_18, GPIO_OUTPUT},
			{GPIO_19, GPIO_OUTPUT}};
	for (uint8_t i = 0; i < 3; i++)
	{
		GPIOInit(GPIO_9_18_19[i].pin, GPIO_9_18_19[i].dir);
	}

	valor = 123;
	cantidadDigitos = 3;
	mostrarDisplay(valor, cantidadDigitos, GPIO_20_23, GPIO_9_18_19);
}
/*==================[end of file]============================================*/