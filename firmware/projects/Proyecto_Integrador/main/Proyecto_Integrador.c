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

#include "neopixel_stripe.h"
#include "ble_mcu.h"
#include <string.h>

#include "servo_sg90.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_ANALOGICO_US 	5000   // con 500 tengo frecuencia de 2kHz, con 1000us tengo 1kHz, para 1500Hz t=666
#define CONFIG_BLINK_PERIOD 		500
#define FRECUENCIA_PASA_ALTO 		20
#define FRECIENCIA_PASA_BAJO		5
#define SIZE_EMG 					256
#define CHUNK               		4
#define SIZE_EMG_COMPLETO			512

/*==================[internal data definition]===============================*/
TaskHandle_t Leer_Procesar_Analogico_task_handle = NULL; 
TaskHandle_t fft_task_handle = NULL;

uint16_t umbral_0 = 1;
static float emg [CHUNK];
static float emg_filtrado [CHUNK] ;
//static float emg_completo [SIZE_EMG_COMPLETO];

// float emg_filtrado [SIZE_EMG];
static float emg_completo [SIZE_EMG_COMPLETO];
bool procesar = false;
uint16_t contador_emg_completo = 0;
/*==================[internal functions declaration]=========================*/


/**
 * @brief procesa el emg completo y muevo el servo
 */
// void procesar()
// {
//		
// };


/**
 * @brief Función invocada en la interrupción del timer A, para convertir a analogico
 */
// void FuncTimerConvertirAnalogico(void* param)
// {
//     vTaskNotifyGiveFromISR(Leer_Procesar_Analogico_task_handle, pdFALSE);    
// }

/**
 * @brief Tarea encargada de leer el dato analogico y guardarlo en la variable general lectura 
 */


// static void task_Leer_Procesar_Analogico(void *pvParameter)
// {
// 	uint8_t contador = 0; 
// 	while (true)
// 	{
// 		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
// 	}
// };


/**
 * @brief Función invocada en la interrupción del timer B
 */
void FuncTimerSenial(void* param)
{
    xTaskNotifyGive(fft_task_handle);
}


static void FftTask(void *pvParameter)
{
	uint16_t lectura = 0;
    char msg[128];
    char msg_chunk[24];
    uint16_t indice = 0;
//	uint8_t contador = 0; 
    while(true)
	{
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		// tengo que contar las 4 veces que lee y depues de las 4 activo el filtro

		AnalogInputReadSingle(CH1, &lectura);
		emg[indice] = (float) lectura; 
		emg_completo[contador_emg_completo] = (float) lectura;
		if(indice == CHUNK)
		{
            //HiPassFilter(&emg[0], emg_filtrado, CHUNK);
			//LowPassFilter(emg_filtrado, emg_filtrado, CHUNK);
			for (uint8_t i = 0; i < CHUNK; i++)
			{
				//UartSendString(UART_PC, (const char* )  UartItoa( emg_filtrado[i], 10)); 
				//UartSendString(UART_PC, (const char* )  UartItoa( lectura, 10)); 
				//UartSendString(UART_PC, "\r");
			}
			indice = 0;
		}
		indice ++;
		// UartSendString(UART_PC, (const char* )  UartItoa( emg_filtrado[indice + contador], 10)); 
		// UartSendString(UART_PC, "\r");
		
		//else
		//{
            //memcpy(emg_filtrado, &emg[indice], CHUNK*sizeof(float));
        //}

		// for(uint8_t i = 0; i < CHUNK; i++ )
		// {
		// 	emg_completo[contador_emg_completo] = emg_filtrado[i];
		// 	contador_emg_completo++;
		// }
		// if(contador_emg_completo == SIZE_EMG_COMPLETO)
		// {
		// 	procesar();
		// 	contador_emg_completo = 0; 
		// }

		//mandar por bl desde aca 

        strcpy(msg, "");
        for(uint8_t i=0; i<CHUNK; i++)
		{
            sprintf(msg_chunk, "*G%.2f*", emg_filtrado[i]);
            strcat(msg, msg_chunk);
        }
		BleSendString(msg);	

		// hasta ahi

		if (contador_emg_completo < 512)
		{
			procesar = true; 
			contador_emg_completo = 0;
		}
		contador_emg_completo++;
    }
}


/*==================[external functions definition]==========================*/

void app_main(void)
{
	analog_input_config_t analogico_i =	{	
		.input = CH1, 	
		.mode = ADC_SINGLE,	
		.func_p = NULL, 	
		.param_p = NULL 
	};
	AnalogInputInit(&analogico_i);


	// timer_config_t timerAnalogico = {    
	// 	.timer = TIMER_A,    
	// 	.period = CONFIG_PERIOD_ANALOGICO_US,    
	// 	.func_p = FuncTimerConvertirAnalogico,    
	// 	.param_p = NULL 
	// };

	// TimerInit(&timerAnalogico);

	serial_config_t miUart = {
		.port =  UART_PC,	
		.baud_rate = 115200,		
		.func_p = NULL,			
		.param_p = NULL
	}; 

	UartInit(&miUart);
	AnalogOutputInit();

	// xTaskCreate(&task_Leer_Procesar_Analogico, "Leer y Procesar Analogico", 4096, NULL, 5, &Leer_Procesar_Analogico_task_handle);
	// TimerStart(timerAnalogico.timer);


    timer_config_t timer_senial = {
        .timer = TIMER_B,
        .period = CONFIG_PERIOD_ANALOGICO_US,
        .func_p = FuncTimerSenial,
        .param_p = NULL
    };
 	TimerInit(&timer_senial);

   

    LowPassInit(FRECIENCIA_PASA_BAJO, 45, ORDER_2);
    HiPassInit(FRECUENCIA_PASA_ALTO, 1, ORDER_2);
	//HiPassInit(FRECUENCIA_PASA_ALTO, 1, ORDER_2);

 

   xTaskCreate(&FftTask, "FFT", 4096, NULL, 5, &fft_task_handle);
   TimerStart(timer_senial.timer);


	int8_t ang=90;
	ServoInit(SERVO_0,CH2);
	//ServoMove(SERVO_0, ang);

	while (1)
	{
		ServoMove(SERVO_0, ang);
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}





	// uint8_t blink = 0;
   //  static neopixel_color_t color;
    // ble_config_t ble_configuration = {
    //     "ESP_EDU_ALDU",
    //     NULL
    // };

 	// NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
    // NeoPixelAllOff();
  // BleInit(&ble_configuration);
	// while(1)
	// {
    //     vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
    //     switch(BleStatus())
	// 	{
    //         case BLE_OFF:
    //             NeoPixelAllOff();
    //         break;
    //         case BLE_DISCONNECTED:
    //             if(blink%2)
	// 			{
    //                 NeoPixelAllColor(NEOPIXEL_COLOR_TURQUOISE);
    //             }
	// 			else
	// 			{
    //                 NeoPixelAllOff();
    //             }
    //             blink++;
    //         break;
    //         case BLE_CONNECTED:
    //             NeoPixelAllColor(NEOPIXEL_COLOR_ROSE);
    //         break;
    //     }
    // }

};
/*==================[end of file]============================================*/