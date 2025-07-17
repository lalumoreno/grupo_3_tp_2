#include "main.h"
#include "cmsis_os.h"
#include "task_button.h"
#include "task_ui.h"
#include "task_led.h"
#include "task_uart.h"
#include "board.h"

/* Definición de colas */
QueueHandle_t uart_queue;
QueueHandle_t button_event_queue;

led_t leds[3];
const int led_count = sizeof(leds) / sizeof(led_t);

void app_init(void) {
	/* Crear cola UART */
	uart_queue = xQueueCreate(5, sizeof(char[64]));
	configASSERT(uart_queue != NULL);
	if (uart_queue == NULL) {
		// Fallback directo si falla
		while (1)
			;
	}

	/* Crear cola de eventos del botón */
	button_event_queue = xQueueCreate(1, sizeof(button_event_t *));
	configASSERT(button_event_queue != NULL);
	if (button_event_queue == NULL) {
		log_uart("Error: no se pudo crear la cola de botón\r\n");
		while (1)
			;
	}

	// Crear cola de leds
	for (int i = 0; i < led_count; i++) {
		leds[i].queue = xQueueCreate(5, sizeof(led_event_t *));
	}

	/* Crear tareas del sistema */
	BaseType_t status;

	status = xTaskCreate(task_uart, "task_uart", 128, NULL,
	tskIDLE_PRIORITY + 1, NULL);
	configASSERT(status == pdPASS);

	status = xTaskCreate(task_button, "task_button", 512, NULL,
	tskIDLE_PRIORITY + 2, NULL);
	configASSERT(status == pdPASS);

	status = xTaskCreate(task_ui, "task_ui", 128, (void*) leds,
	tskIDLE_PRIORITY + 2, NULL);
	configASSERT(status == pdPASS);

	status = xTaskCreate(task_led, "task_led", 128, (void*) leds,
	tskIDLE_PRIORITY + 1, NULL);
	configASSERT(status == pdPASS);

	/* Enviar mensaje por UART */
	log_uart("APP → App init\r\n");

#ifdef _F429ZI_
	log_uart("APP → Board F429ZI\r\n");
#endif

}
