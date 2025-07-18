#include "cmsis_os.h"
#include "board.h"
#include "task_button.h"
#include "task_uart.h"
#include "task_led.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* Cola de eventos del botón y leds*/
extern QueueHandle_t button_event_queue;

void task_ui(void *argument) {

	led_t *leds = (led_t*) argument;
	button_event_t *button_event;

	while (1) {

		if (xQueueReceive(button_event_queue, (void*) &button_event,
		portMAX_DELAY) == pdTRUE) {

			bool valid_event = true;
			led_event_t *led_event = (led_event_t*) pvPortMalloc(
					sizeof(led_event_t));
			if (led_event != NULL) {

				char msg[64];
				sprintf(msg, "UI → Memoria led_event alocada: %d \r\n",
						sizeof(*led_event));
				log_uart(msg);

				switch (button_event->type) {
				case BUTTON_TYPE_PULSE:
					log_uart("UI → Boton tipo PULSO - activar LED rojo\r\n");
					led_event->type = LED_EVENT_RED;
					break;

				case BUTTON_TYPE_SHORT:
					log_uart("UI → Boton tipo CORTO - activar LED verde\r\n");
					led_event->type = LED_EVENT_GREEN;
					break;

				case BUTTON_TYPE_LONG:
					log_uart("UI → Boton tipo LARGO - activar LED azul\r\n");
					led_event->type = LED_EVENT_BLUE;
					break;

				default:
					log_uart("UI → tipo de pulsacion no reconocido\r\n");
					valid_event = false;
					break;
				}

				if (valid_event) {
					BaseType_t sent = xQueueSend(leds[led_event->type].queue,
							&led_event, portMAX_DELAY);
					if (sent != pdPASS) {
						log_uart(
								"UI → Error agregando evento a la cola led_event_queue\r\n");
						vPortFree(led_event);
						log_uart("UI → Memoria de led_event liberada \r\n");

					}
				}

				vPortFree(button_event);
				log_uart("UI → Memoria button_event liberada \r\n");
			} else {
				log_uart("BTN → Memoria insuficiente\r\n");
			}
		}
	}
}
