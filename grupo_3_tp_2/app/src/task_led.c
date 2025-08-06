#include <string.h>
#include "task_led.h"
#include "main.h"
#include "cmsis_os.h"
#include "task_uart.h"
#include "board.h"

typedef enum
{
	LED_STATE_STANDBY,
	LED_STATE_RED,
	LED_STATE_GREEN,
	LED_STATE_BLUE
} led_state_t;

TaskHandle_t task_led_handle = NULL;

static void leds_off()
{
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_RESET);
	log_uart("LED - LEDs apagados\r\n");
}

static void led_red_on()
{
	HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
}

static void led_green_on()
{
	HAL_GPIO_WritePin(LED_GREEN_PORT, LED_GREEN_PIN, GPIO_PIN_SET);
}

static void led_blue_on()
{
	HAL_GPIO_WritePin(LED_BLUE_PORT, LED_BLUE_PIN, GPIO_PIN_SET);
}

bool add_led_event_to_queue(led_t *leds, led_event_type_t event_type, led_event_t *event)
{
	if (leds[event_type].queue == NULL)
	{
		log_uart("LED - Cola del LED no inicializada\r\n");
		return false;
	}

	BaseType_t sent = xQueueSend(leds[event_type].queue, &event, portMAX_DELAY);
	if (sent != pdPASS)
	{
		log_uart("LED - Error agregando evento a la cola del LED\r\n");
		return false;
	}

	log_uart("LED - Evento agregado a la cola del LED\r\n");
	return true;
}

static void destroy_task()
{
	log_uart("LED - Destruir tarea task_led_handle\r\n");
	task_led_handle = NULL;
	vTaskDelete(NULL);
}

void task_led(void *argument)
{

	led_t *leds = (led_t *)argument;
	led_event_t *led_event;

	leds_off();

	while (1)
	{
		for (int i = 0; i < 3; i++)
		{
			if (leds[i].queue != NULL)
			{
				if (xQueueReceive(leds[i].queue, (void *)&led_event,
								  0) == pdTRUE)
				{
					// Apagar todos los LEDs
					leds_off();

					switch (led_event->type)
					{
					case LED_EVENT_RED:
						log_uart("LED - Encender LED Rojo\r\n");
						led_red_on();
						break;
					case LED_EVENT_GREEN:
						log_uart("LED - Encender LED Verde\r\n");
						led_green_on();
						break;
					case LED_EVENT_BLUE:
						log_uart("LED - Encender LED Azul\r\n");
						led_blue_on();
						break;
					default:
						log_uart("LED - Estado Desconocido\r\n");
						break;
					}

					log_uart("LED - Evento led_event procesado \r\n");
					if (led_event->callback_process_completed != NULL)
					{
						led_event->callback_process_completed(led_event);
					}
					else
					{
						log_uart("LED - led_event callback vacio\r\n");
					}
				}

				// Si no hay eventos, destruir la cola
				log_uart("UUI - Destruir cola de led\r\n");
				vQueueDelete(leds[i].queue);
				leds[i].queue = NULL;
			}
		}

		// Si no hay eventos, destruir la tarea
		destroy_task();
	}
}
