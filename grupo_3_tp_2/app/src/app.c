#include "main.h"
#include "cmsis_os.h"
#include "task_button.h"
#include "task_ui.h"
#include "task_led.h"
#include "task_uart.h"
#include "board.h"

/* Definición de colas */
QueueHandle_t uart_queue;

void app_init(void) {

	/* Crear cola UART */
	uart_queue = xQueueCreate(20, sizeof(char*));
	configASSERT(uart_queue != NULL);
	if (uart_queue == NULL) {
		// Fallback directo si falla
		while (1)
			;
	}

	task_ui_init();
	
	/* Crear tareas del sistema */
	BaseType_t status;
	status = xTaskCreate(task_uart, "task_uart", 128, NULL,
	tskIDLE_PRIORITY + 1, NULL);
	configASSERT(status == pdPASS);

	status = xTaskCreate(task_button, "task_button", 128, NULL,
	tskIDLE_PRIORITY + 2, NULL);
	configASSERT(status == pdPASS);

	/* Enviar mensaje por UART */
	log_uart("APP - App init\r\n");

#ifdef _F429ZI_
	log_uart("APP - Board F429ZI\r\n");
#endif

}
