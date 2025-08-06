#include "cmsis_os.h"
#include "main.h"
#include "task_button.h"
#include "task_uart.h"
#include "board.h"
#include "task_ui.h"
#include "task_led.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#define TASK_BUTTON_STACK_SIZE 128
#define TASK_BUTTON_PRIORITY (tskIDLE_PRIORITY + 1)
#define TASK_PERIOD_MS 50
#define BUTTON_PULSE_TIMEOUT 200
#define BUTTON_SHORT_TIMEOUT 1000
#define BUTTON_LONG_TIMEOUT 2000

/* Estados internos del botón */
typedef enum
{
	BUTTON_STATE_IDLE,
	BUTTON_STATE_PRESSED
} button_state_t;

typedef struct
{
	uint32_t counter;
	button_state_t state;
} button_info_t;

static button_info_t button_info;

static void task_button(void *argument);
static button_event_t button_process_state(bool is_pressed);
static void callback_process_completed(void *context);

/* Clasifica la pulsación */
static button_event_t button_process_state(bool is_pressed)
{

	button_event_t event = {BUTTON_TYPE_NONE, 0};

	if (is_pressed)
	{
		log_uart("BTN - Estado del boton: PRESIONADO\r\n");
		button_info.counter += TASK_PERIOD_MS;
		button_info.state = BUTTON_STATE_PRESSED;
	}
	else
	{
		if (button_info.state == BUTTON_STATE_PRESSED)
		{
			event.duration = button_info.counter;

			if (button_info.counter >= BUTTON_LONG_TIMEOUT)
			{
				event.type = BUTTON_TYPE_LONG;
			}
			else if (button_info.counter >= BUTTON_SHORT_TIMEOUT)
			{
				event.type = BUTTON_TYPE_SHORT;
			}
			else if (button_info.counter >= BUTTON_PULSE_TIMEOUT)
			{
				event.type = BUTTON_TYPE_PULSE;
			}
			else
			{
				event.type = BUTTON_TYPE_NONE;
			}
		}

		button_info.counter = 0;
		button_info.state = BUTTON_STATE_IDLE;
	}

	return event;
}

static void callback_process_completed(void *context)
{
	button_event_t *event = (button_event_t *)context;
	vPortFree(event);
	log_uart("BTN - Memoria bnt_event liberada desde callback\r\n");
}

void init_task_button(void)
{
	BaseType_t status = xTaskCreate(task_button, "task_button", TASK_BUTTON_STACK_SIZE, NULL, TASK_BUTTON_PRIORITY, NULL);
	configASSERT(status == pdPASS);
	if (status != pdPASS)
	{
		log_uart("Error: No se pudo crear la tarea Button\r\n");
	}
	else
	{
		log_uart("Tarea Button creada correctamente\r\n");
	}
}

/* Tarea del botón (modo polling) */
static void task_button(void *argument)
{
	button_info.counter = 0;
	button_info.state = BUTTON_STATE_IDLE;

	while (1)
	{
		GPIO_PinState state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
#ifdef _F429ZI_
		bool is_pressed = (state == GPIO_PIN_SET);
#else
		bool is_pressed = (state == GPIO_PIN_RESET);
#endif

		button_event_t temp_event = button_process_state(is_pressed);

		if (temp_event.type != BUTTON_TYPE_NONE)
		{

			/* Crear tareas ui y led para procesar evento. Cada tarea se autodestruye */
			create_ui_task(temp_event.type);

			// Asignar memoria para el evento
			button_event_t *bnt_event = (button_event_t *)pvPortMalloc(
				sizeof(button_event_t));

			if (bnt_event != NULL)
			{
				char msg[UART_MSG_MAX_LEN];
				sprintf(msg, "UUI - Memoria bnt_event alocada: %d\r\n",
						sizeof(*bnt_event));
				log_uart(msg);

				*bnt_event = temp_event;
				bnt_event->callback_process_completed = callback_process_completed;
				bnt_event->callback_context = bnt_event;

				// Mostrar mensaje por UART
				sprintf(msg, "BTN - Boton: %s - Tiempo: %ld ms\r\n",
						(bnt_event->type == BUTTON_TYPE_LONG) ? "LARGO" : (bnt_event->type == BUTTON_TYPE_SHORT) ? "CORTO"
																												 : "PULSO",
						bnt_event->duration);
				log_uart(msg);

				// Enviar evento a la cola
				if (!add_event_to_ui_queue(bnt_event))
				{
					vPortFree(bnt_event); // Liberar si no se pudo enviar
					log_uart("BTN - Memoria de bnt_event liberada \r\n");
				}
			}
			else
			{
				log_uart("BTN - Memoria insuficiente\r\n");
			}
		}

		vTaskDelay(pdMS_TO_TICKS(TASK_PERIOD_MS));
	}
}
