#include "cmsis_os.h"
#include "board.h"
#include "task_uart.h"
#include "task_led.h"
#include "task_ui.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

/* Cola de eventos del botón*/
QueueHandle_t button_event_queue;
TaskHandle_t task_ui_handle = NULL;

void task_ui_init()
{
	/* Crear cola de eventos del botón */
	button_event_queue = xQueueCreate(1, sizeof(button_event_t *));
	configASSERT(button_event_queue != NULL);
	if (button_event_queue == NULL)
	{
		log_uart("Error: no se pudo crear la cola de botón\r\n");
		while (1)
			;
	}
}

bool add_button_event_to_queue(button_event_t *event)
{
	if (button_event_queue == NULL)
	{
		log_uart("UUI - Cola de eventos del botón no inicializada\r\n");
		return false;
	}

	BaseType_t sent = xQueueSend(button_event_queue, &event, portMAX_DELAY);
	if (sent != pdPASS)
	{
		log_uart("UUI - Error agregando evento a la cola button_event_queue\r\n");
		return false;
	}

	log_uart("UUI - Evento agregado a la cola button_event_queue\r\n");
	return true;
}

bool receive_button_event_from_queue(button_event_t **event)
{
	if (button_event_queue == NULL)
	{
		log_uart("UUI - Cola de eventos del botón no inicializada\r\n");
		return false;
	}

	if (xQueueReceive(button_event_queue, event, portMAX_DELAY) == pdTRUE)
	{
		log_uart("UUI - Evento recibido de la cola button_event_queue\r\n");
		return true;
	}
	else
	{
		log_uart("UUI - Error al recibir evento de la cola button_event_queue\r\n");
		return false;
	}
}

static void callback_process_completed_(void *context)
{
	led_event_t *event = (led_event_t *)context;
	vPortFree(event);
	log_uart("UUI - Memoria led_event liberada\r\n");
}

static void destroy_task()
{
	log_uart("UUI - Destruir tarea task_ui_handle\r\n");
	task_ui_handle = NULL;
	vTaskDelete(NULL);
}

void task_ui(void *argument)
{

	led_t *leds = (led_t *)argument;
	button_event_t *button_event;

	while (1)
	{
		if (receive_button_event_from_queue(&button_event) == false)
		{
			log_uart("UUI - No se pudo recibir evento de la cola button_event_queue\r\n");
			destroy_task();
			return;
		}

		bool valid_event = true;
		led_event_t *led_event = (led_event_t *)pvPortMalloc(
			sizeof(led_event_t));

		if (led_event != NULL)
		{
			char msg[64];
			sprintf(msg, "UUI - Memoria led_event alocada: %d\r\n",
					sizeof(*led_event));
			log_uart(msg);

			led_event->callback_process_completed = callback_process_completed_;
			led_event->callback_context = led_event;

			switch (button_event->type)
			{
			case BUTTON_TYPE_PULSE:
				log_uart("UUI - Boton tipo PULSO - activar LED rojo\r\n");
				led_event->type = LED_EVENT_RED;
				break;

			case BUTTON_TYPE_SHORT:
				log_uart("UUI - Boton tipo CORTO - activar LED verde\r\n");
				led_event->type = LED_EVENT_GREEN;
				break;

			case BUTTON_TYPE_LONG:
				log_uart("UUI - Boton tipo LARGO - activar LED azul\r\n");
				led_event->type = LED_EVENT_BLUE;
				break;

			default:
				log_uart("UUI - tipo de pulsacion no reconocido\r\n");
				valid_event = false;
				break;
			}

			if (valid_event)
			{
				if (!add_led_event_to_queue(leds, led_event->type, led_event))
				{
					log_uart("UUI - Error al agregar evento a la cola del LED\r\n");
					vPortFree(led_event);
					log_uart("UUI - Memoria de led_event liberada \r\n");
				}
			}

			log_uart("UUI - Evento button_event procesado \r\n");
			if (button_event->callback_process_completed != NULL)
			{
				button_event->callback_process_completed(button_event);
			}
			else
			{
				log_uart("UUI - button_event callback vacio\r\n");
			}
		}
		else
		{
			log_uart("UUI - Memoria insuficiente\r\n");
		}
	}

	// Si no hay eventos, destruir la tarea
	destroy_task();
}
