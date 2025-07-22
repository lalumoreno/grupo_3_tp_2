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

/* Cola compartida con task_ui */
extern QueueHandle_t button_event_queue;

/* Tareas*/
extern TaskHandle_t task_led_handle;
extern TaskHandle_t task_ui_handle;

led_t leds[3];

/* Tiempos */
#define TASK_PERIOD_MS        50
#define BUTTON_PULSE_TIMEOUT  200
#define BUTTON_SHORT_TIMEOUT  1000
#define BUTTON_LONG_TIMEOUT   2000

/* Estados internos del botón */
typedef enum {
	BUTTON_STATE_IDLE, BUTTON_STATE_PRESSED
} button_state_t;

typedef struct {
	uint32_t counter;
	button_state_t state;
} button_info_t;

static button_info_t button_info;

/* Inicializa el estado del botón */
static void button_init_(void) {
	button_info.counter = 0;
	button_info.state = BUTTON_STATE_IDLE;
}

/* Clasifica la pulsación */
static button_event_t button_process_state_(bool is_pressed) {

	button_event_t event = { BUTTON_TYPE_NONE, 0 };

	if (is_pressed) {
		log_uart("BTN - Estado del boton: PRESIONADO\r\n");
		button_info.counter += TASK_PERIOD_MS;
		button_info.state = BUTTON_STATE_PRESSED;
	} else {
		if (button_info.state == BUTTON_STATE_PRESSED) {
			event.duration = button_info.counter;

			if (button_info.counter >= BUTTON_LONG_TIMEOUT) {
				event.type = BUTTON_TYPE_LONG;
			} else if (button_info.counter >= BUTTON_SHORT_TIMEOUT) {
				event.type = BUTTON_TYPE_SHORT;
			} else if (button_info.counter >= BUTTON_PULSE_TIMEOUT) {
				event.type = BUTTON_TYPE_PULSE;
			} else {
				event.type = BUTTON_TYPE_NONE;
			}
		}

		button_info.counter = 0;
		button_info.state = BUTTON_STATE_IDLE;
	}

	return event;
}

static void callback_process_completed_(void *context) {
	button_event_t *event = (button_event_t*) context;
	vPortFree(event);
	log_uart("BTN - Memoria bnt_event liberada\r\n");
}

void create_ui_led_tasks(button_event_t event) {
	BaseType_t status;
	int led_type = event.type -1;

	// Crear cola de leds
	log_uart("BTN - Crear cola de led\r\n");
	if (leds[led_type].queue == NULL) {
		leds[led_type].queue = xQueueCreate(5, sizeof(led_event_t*));
	}

	if (task_ui_handle == NULL) {
		log_uart("BTN - Crear tarea task_ui_handle\r\n");
		status = xTaskCreate(task_ui, "task_ui", 128, (void*) leds,
		tskIDLE_PRIORITY + 2, &task_ui_handle);
		configASSERT(status == pdPASS);
	}

	if (task_led_handle == NULL) {
		log_uart("BTN - Crear tarea task_led_handle\r\n");
		status = xTaskCreate(task_led, "task_led", 128, (void*) leds,
		tskIDLE_PRIORITY + 1, &task_led_handle);
		configASSERT(status == pdPASS);
	}
}

/* Tarea del botón (modo polling) */
void task_button(void *argument) {

	button_init_();

	while (1) {
		GPIO_PinState state = HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN);
#ifdef _F429ZI_
		bool is_pressed = (state == GPIO_PIN_SET);
#else
		bool is_pressed = (state == GPIO_PIN_RESET);
#endif

		button_event_t temp_event = button_process_state_(is_pressed);

		if (temp_event.type != BUTTON_TYPE_NONE) {

			/* Crear tareas ui y led para procesar evento. Cada tarea se autodestruye */
			create_ui_led_tasks(temp_event);

			/* Crear y agregar evento a la cola */
			button_event_t *bnt_event = (button_event_t*) pvPortMalloc(
					sizeof(button_event_t));

			if (bnt_event != NULL) {
				char msg[64];
				sprintf(msg, "UUI - Memoria bnt_event alocada: %d\r\n",
						sizeof(*bnt_event));
				log_uart(msg);

				*bnt_event = temp_event;
				bnt_event->callback_process_completed =
						callback_process_completed_;
				bnt_event->callback_context = bnt_event;

				// Mostrar mensaje por UART
				sprintf(msg, "BTN - Boton: %s - Tiempo: %ld ms\r\n",
						(bnt_event->type == BUTTON_TYPE_LONG) ? "LARGO" :
						(bnt_event->type == BUTTON_TYPE_SHORT) ?
								"CORTO" : "PULSO", bnt_event->duration);
				log_uart(msg);

				// Enviar evento a la cola
				BaseType_t sent = xQueueSend(button_event_queue, &bnt_event, 0);
				if (sent != pdPASS) {
					log_uart(
							"BTN - Error agregando evento a la cola button_event_queue\r\n");
					vPortFree(bnt_event);  // Liberar si no se pudo enviar
					log_uart("BTN → Memoria de bnt_event liberada \r\n");
				}
			} else {
				log_uart("BTN - Memoria insuficiente\r\n");
			}
		}

		vTaskDelay(pdMS_TO_TICKS(TASK_PERIOD_MS));
	}
}
