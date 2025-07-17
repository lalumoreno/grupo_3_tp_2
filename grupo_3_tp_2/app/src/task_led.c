#include <string.h>
#include "task_led.h"
#include "main.h"
#include "cmsis_os.h"
#include "task_uart.h"
#include "board.h"

typedef enum {
	LED_STATE_STANDBY, LED_STATE_RED, LED_STATE_GREEN, LED_STATE_BLUE
} led_state_t;

extern osMessageQId QueueUartHandle;  // Cola de UART

void leds_off() {
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_RESET);
	log_uart("LED → LEDs apagados\r\n");
}

void led_red_on() {
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
}

void led_green_on() {
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
}

void led_blue_on() {
	HAL_GPIO_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_SET);
}

void task_led(void *argument) {

	led_t *leds = (led_t*) argument;
	led_event_t *led_event;

	leds_off();

	while (1) {
		for (int i = 0; i < 3; i++) {
			if (xQueueReceive(leds[i].queue, (void*)&led_event, 0) == pdTRUE) {
				// Apagar todos los LEDs
				leds_off();

				switch (led_event->type) {
				case LED_EVENT_RED:
					log_uart("LED → Encender LED Rojo\r\n");
					led_red_on();
					break;
				case LED_EVENT_GREEN:
					log_uart("LED → Encender LED Verde\r\n");
					led_green_on();
					break;
				case LED_EVENT_BLUE:
					log_uart("LED → Encender LED Azul\r\n");
					led_blue_on();
					break;
				default:
					log_uart("LED → Estado Desconocido\r\n");
					break;
				}

				vPortFree(led_event);
				log_uart("LED → Memoria led_event liberada \r\n");
			}
		}

		vTaskDelay(pdMS_TO_TICKS(10));
	}
}
