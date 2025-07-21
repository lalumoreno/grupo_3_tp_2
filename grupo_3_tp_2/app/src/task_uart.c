#include "task_uart.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>  // por si usas sprintf o snprintf
#include "board.h"

extern UART_HandleTypeDef huart2;
extern QueueHandle_t uart_queue;  // Definida en app.c

#define UART_MSG_MAX_LEN 64

void task_uart(void *argument) {
	char *msg;

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
