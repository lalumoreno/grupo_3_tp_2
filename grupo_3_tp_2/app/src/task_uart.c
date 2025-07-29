#include "task_uart.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>  // por si usas sprintf o snprintf
#include "board.h"

extern UART_HandleTypeDef huart2;

#define UART_MSG_MAX_LEN 64

static QueueHandle_t uart_queue = NULL;

void task_uart(void *argument) {
	char *msg;

	/* Crear cola UART */
	uart_queue = xQueueCreate(20, sizeof(char*));
	configASSERT(uart_queue != NULL);
	if (uart_queue == NULL) {
		// Fallback directo si falla
		while (1) {
			// En caso de error, encender el LED rojo y esperar
			HAL_GPIO_WritePin(LED_RED_PORT, LED_RED_PIN, GPIO_PIN_SET);
		}
	}

	/* Inicializar UART */
	while (1) {
		if (xQueueReceive(uart_queue, &msg, portMAX_DELAY) == pdTRUE) {
			HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
					HAL_MAX_DELAY);
			vPortFree(msg);  // Libera la memoria asignada en log_uart
		}
	}
}

/**
 * @brief Envía un mensaje por UART de forma segura a través de la cola
 * @param msg Cadena null-terminated (máx. 63 caracteres)
 */
void log_uart(const char *msg) {
	if (msg == NULL)
		return;

	if (uart_queue == NULL) {
		// Si la cola no está creada, no se puede enviar el mensaje
		return;
	}
	
	size_t len = strnlen(msg, UART_MSG_MAX_LEN - 1);
	char *copy = pvPortMalloc(len + 1);
	if (copy == NULL)
		return;

	strncpy(copy, msg, len);
	copy[len] = '\0';

	BaseType_t sent = xQueueSend(uart_queue, &copy, 0);
	if (sent != pdPASS) {
		vPortFree(copy);  // libera si no se pudo enviar
	}
}
