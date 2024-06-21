/*! @mainpage Proyecto_Integrador
 *
 * @author Aldana Agustina Fernandez (aldana.fernandez@ingenieria.uner.edu.ar)
 *
 * @section Proyecto_Integrador
 *
 * @section Descripcion General
 *
 * Control de prótesis para mano mediante electromiografía.
 *
 *
 * @section hardConn Hardware Connection
 *
 * |  	   	    Perifericos		    |   ESP32  	|
 * |:------------------------------:|:----------|
 * |Placa biopotenciales alimetacion|   5V		|
 * |Placa biopotenciales EMG 		|   CH1		|
 * |Placa biopotenciales GND		|   GND		|
 * |SERVO alimentacion			 	|   3,3V	|
 * |SERVO PWM	 					|   GPIO2	|
 * |SERVO GND					 	|   GND		|
 *
 *
 * @section changelog Changelog
 *
 * |   Fecha	| Descripcion           |
 * |:----------:|:----------------------|
 * | 17/05/2024 | Document creation		|
 * | 19/06/2024 | Document completion	|
 *
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <math.h>
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
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_PERIOD_SENIAL_US 5000
#define SAMPLE_FREQ 200
#define CONFIG_PERIOD_PROCESAR_US 2560000
#define CONFIG_BLINK_PERIOD 500
#define FRECUENCIA_PASA_ALTO 40
#define FRECIENCIA_PASA_BAJO 1
#define CHUNK 4
#define SIZE_EMG_COMPLETO 512
/*==================[internal data definition]===============================*/
TaskHandle_t senial_task_handle = NULL;
TaskHandle_t mover_task_handle = NULL;
volatile float emg_valor_absoluto[SIZE_EMG_COMPLETO]; 	// Arreglo que se usa para guardar 512 valores de emg filtrado y rectificado
/*==================[internal functions declaration]=========================*/

/**
 * @brief Función invocada en la interrupción del timer A, para realizar la lectura del EMG
 */
void FuncTimerSenial(void *param)
{
	xTaskNotifyGive(senial_task_handle);
}

/**
 * @brief Función invocada en la interrupción del timer B, para mover el servo
 */
void FuncTimerMover(void *param)
{
	xTaskNotifyGive(mover_task_handle);
}

/**
 * @brief Función invocada en la interrupción del timer A, para realizar la lectura del EMG, aplicar los filtros, rectificarla y mandar por bluetooth.
 */
static void TaskSenial(void *pvParameter)
{
	float emg[CHUNK] = {0};						  	  // Arreglo que se usa para guardar de a 4 lecturas de emg
	float emg_filtrado[CHUNK] = {0};				  // Arreglo que se usa para guardar de a 4 valores de emg filtrado
	uint16_t contador_emg_completo = 0;
	uint16_t lectura = 0;
	char msg[128];
	char msg_chunk[24];
	char msg_filtrado[128];
	char msg_chunk_filtrado[24];
	uint16_t indice = 0;
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &lectura);
		emg[indice] = (float)lectura;
		indice++;
		if (indice == CHUNK)
		{
			HiPassFilter(&emg[0], emg_filtrado, CHUNK);
			LowPassFilter(emg_filtrado, emg_filtrado, CHUNK);
			strcpy(msg, "");
			strcpy(msg_filtrado, "");
			for (uint8_t i = 0; i < CHUNK; i++)
			{
				emg_valor_absoluto[i + contador_emg_completo] = fabs(emg_filtrado[i]);
				sprintf(msg_chunk, "*A%.2f*", emg[i]);
				strcat(msg, msg_chunk);
				sprintf(msg_chunk_filtrado, "*G%.2f*", emg_filtrado[i]);
				strcat(msg_filtrado, msg_chunk_filtrado);
			}
			BleSendString(msg_filtrado);
			BleSendString(msg);
			indice = 0;
			contador_emg_completo = contador_emg_completo + CHUNK;
		}
		if (contador_emg_completo >= SIZE_EMG_COMPLETO)
		{
			contador_emg_completo = 0;
		}
	}
}

/**
 * @brief Tarea encargada de verificar que si se pasa el valor umbral se mueva el servo
 */
static void TaskMover(void *pvParameter)
{
	bool abrir = false;				// Indica si se abrio o cerro el servo
	float maximo = 0;
	uint8_t angulo = 90;
	float umbral = 0.05;   
	while (true)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		maximo = 0;
		for (int j = 0; j < SIZE_EMG_COMPLETO; j++)
		{
			if (maximo < emg_valor_absoluto[j])
			{
				maximo = emg_valor_absoluto[j];
			}
		}
		if (maximo >= umbral)
		{
			if (abrir)
			{
				ServoMove(SERVO_0, angulo);
				printf("abrir mano\n");
				abrir = false;
			}
			else
			{
				ServoMove(SERVO_0, angulo*(-1) );
				printf("cerrar mano\n");
				abrir = true;
			}
		}
	}
}

/*==================[external functions definition]==========================*/

void app_main(void)
{
	ServoInit(SERVO_0, CH2);
	analog_input_config_t analogico_i = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL};
	AnalogInputInit(&analogico_i);

	AnalogOutputInit();

	timer_config_t timer_senial = {
		.timer = TIMER_A,
		.period = CONFIG_PERIOD_SENIAL_US,
		.func_p = FuncTimerSenial,
		.param_p = NULL};
	TimerInit(&timer_senial);

	timer_config_t timer_mover = {
		.timer = TIMER_B,
		.period = CONFIG_PERIOD_PROCESAR_US,
		.func_p = FuncTimerMover,
		.param_p = NULL};
	TimerInit(&timer_mover);

	LowPassInit(SAMPLE_FREQ, FRECIENCIA_PASA_BAJO, ORDER_2);
	HiPassInit(SAMPLE_FREQ, FRECUENCIA_PASA_ALTO, ORDER_2);

	xTaskCreate(&TaskSenial, "Senial", 4096, NULL, 5, &senial_task_handle);
	TimerStart(timer_senial.timer);

	xTaskCreate(&TaskMover, "Mover", 4096, NULL, 5, &mover_task_handle);
	TimerStart(timer_mover.timer);

	uint8_t blink = 0;
	static neopixel_color_t color;
	ble_config_t ble_configuration = {
		"ESP_ALDU",
		NULL};

	NeoPixelInit(BUILT_IN_RGB_LED_PIN, BUILT_IN_RGB_LED_LENGTH, &color);
	NeoPixelAllOff();
	BleInit(&ble_configuration);
	while (1)
	{
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
		switch (BleStatus())
		{
		case BLE_OFF:
			NeoPixelAllOff();
			break;
		case BLE_DISCONNECTED:
			if (blink % 2)
			{
				NeoPixelAllColor(NEOPIXEL_COLOR_TURQUOISE);
			}
			else
			{
				NeoPixelAllOff();
			}
			blink++;
			break;
		case BLE_CONNECTED:
			NeoPixelAllColor(NEOPIXEL_COLOR_ROSE);
			break;
		}
	}
};
/*==================[end of file]============================================*/