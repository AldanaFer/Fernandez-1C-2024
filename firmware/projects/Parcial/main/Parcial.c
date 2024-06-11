/*! @mainpage Parcial
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section Parcial
 *
 * @section Descripcion General
 *  El sistema está compuesto por una serie de recipientes con agua, una solución de 
 *  pH ácida y otra básica, un sensor de húmedad y uno de pH, y tres bombas
 *  peristálticas para los líquidos. Hay que encenderlas o apagarlas segun lo que 
 *  devuelvan los sensores. 
 *  Ademas hay que mostrar la informacion por UART.
 *  Y mediente los switch 1 y 2 de la placa controlar si el sistema esta encendido
 *  o apagado.
 * 
 * 
 * 
 * @section hardConn Hardware Connection
 *
 * |   Perifericos  |   ESP32   	|
 * |:--------------:|:--------------|
 * | sensor_pH      | 	CH1     	|
 * | sensor_riego 	| 	GPIO_20		|
 * | bomba_agua 	| 	GPIO_21		|
 * | bomba_pH_acido	| 	GPIO_22		|
 * | bomba_pH_basico| 	GPIO_23		|
 * 
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 11/06/2024 | Document creation		                         |
 * | 11/06/2024 | Document completion		                     |
 *
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "timer_mcu.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_MOSTRAR_US 5000000
#define CONFIG_PERIOD_MEDIR_US 3000000
#define PERIODO_SWITCH 40
/*==================[internal data definition]===============================*/
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t medir_task_handle = NULL;
static bool sistema = true;         // indica si el sistema esta prendido o apagado
static bool bomba_agua = false;     // indica si la bomba de agua esta prendida o apagada
static bool bomba_acido = false;    // indica si la bomba acida esta prendida o apagada
static bool bomba_base = false;     // indica si la bomba basica esta prendida o apagada
float pH;
/*==================[internal functions declaration]=========================*/
/**
 * @brief Función invocada en la interrupción del timer A, para mostrar la informacion
 */
void FuncTimerMostrar(void* param)
{
    vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);    
}

/**
 * @brief Función invocada en la interrupción del timer B, para medir las varaibles
 */
void FuncTimerMedir(void* param)
{
    vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);   
}

/**
 * @brief Tarea encargada de medir las variables y prender o apagar las bombas que corresponden
 */
static void taskMedir(void *pvParameter)
{
	uint16_t dato; 
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        if (sistema)
        {
            AnalogInputReadSingle(CH1, &dato);
            pH = (float)dato * (14.0/3.0) ;  // como el voltaje es de 0 a 3, y 0V es 0 pH, y 3V es 14 pH, el factor de conversion es 14/3
            if(pH < 6.0)
            {
                //prendo bobmba basica y apago la acida
                GPIOOn(GPIO_23);
                GPIOOff(GPIO_22);
                bomba_acido = false;
                bomba_base = true;
            }
            else 
            if(pH > 6.7)
            {
                //prondo bomba acida y apago la basica
                GPIOOn(GPIO_22);
                GPIOOff(GPIO_23);
                bomba_base = false;
                bomba_acido = true;
            }
            else 
            {
                // apago las dos bombas
                GPIOOff(GPIO_22);
                GPIOOff(GPIO_23);
                bomba_acido = false;
                bomba_base = false;
            }
            
            // para la humedad si da 1 hay que prender la bomba si da 0 hay que apagarla
            bomba_agua = GPIORead(GPIO_20);    
            if (bomba_agua)
            {
                GPIOOn(GPIO_20);
            }
            else 
            {
                GPIOOff(GPIO_20);
            }            
        } 
	}
};

/**
 * @brief Tarea encargada de mostrar la informacion a traves de la UART
 */
static void taskMostrar(void *pvParameter)
{
	while (true)
	{
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        if (sistema)
        {
            UartSendString(UART_PC,"pH: ");
            UartSendString(UART_PC, (const char* )  UartItoa( pH, 10)); 

            if(bomba_agua)
            {
               UartSendString(UART_PC,", humedad correcta "); 
            }
            else 
            {
                UartSendString(UART_PC,", humedad incorrecta ");
            }
			UartSendString(UART_PC, "\r\n");

            if(bomba_acido)
            {
                UartSendString(UART_PC,"Bomba de pH acido encendida."); 
            }
            if(bomba_base)
            {
                UartSendString(UART_PC,"Bomba de pH basico encendida."); 
            }

            if(bomba_agua)
            {
                UartSendString(UART_PC,"Bomba de agua encendida."); 
            }
			UartSendString(UART_PC, "\r\n");
        }
	}
};

/**
 * @brief Tarea encargada del manejo de los switch, cambiando el valor del bool sistema.
 */
static void taskSwitch(void *pvParameter)
{
	uint8_t teclas;
	while (true)
	{
		teclas = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
				sistema = true;
    		break;
    		case SWITCH_2:
				sistema = false;
    		break;
    	}
		vTaskDelay(PERIODO_SWITCH / portTICK_PERIOD_MS);
	}
};


/*==================[external functions definition]==========================*/
void app_main(void)
{
		analog_input_config_t sensor_pH =
	{
		.input = CH1, 
		.mode = ADC_SINGLE,
		.func_p = NULL, 
		.param_p = NULL
	};
	AnalogInputInit(&sensor_pH);
	AnalogOutputInit();

    timer_config_t timer_mostrar = {
        .timer = TIMER_A,
        .period = CONFIG_PERIOD_MOSTRAR_US,
        .func_p = FuncTimerMostrar,
        .param_p = NULL
    };
	TimerInit(&timer_mostrar);

	timer_config_t timer_medir = 
	{
   		.timer = TIMER_B, 
        .period = CONFIG_PERIOD_MEDIR_US,
        .func_p = FuncTimerMedir,
        .param_p = NULL
    };
	TimerInit(&timer_medir);

	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 9600,		
		.func_p = NULL,			
		.param_p = NULL
	}; 
	UartInit(&miUart);
    
	xTaskCreate(&taskMostrar, "Mostrar", 4096, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&taskMedir, "Medir", 4096, NULL, 5, &medir_task_handle);
    xTaskCreate(&taskSwitch, "Switch", 1024, NULL, 5, NULL);
	TimerStart(timer_mostrar.timer); 
	TimerStart(timer_medir.timer); 
}
/*==================[end of file]============================================*/