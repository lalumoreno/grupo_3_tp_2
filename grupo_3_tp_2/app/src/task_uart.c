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
	char msg[UART_MSG_MAX_LEN];

	while (1) {
		if (xQueueReceive(uart_queue, &msg, portMAX_DELAY) == pdTRUE) {
			HAL_UART_Transmit(&huart2, (uint8_t*) msg, strlen(msg),
			HAL_MAX_DELAY);
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

	char buffer[UART_MSG_MAX_LEN];
	strncpy(buffer, msg, UART_MSG_MAX_LEN - 1);
	buffer[UART_MSG_MAX_LEN - 1] = '\0';  // Asegura terminación

	// Enviar evento a la cola
	BaseType_t sent = xQueueSend(uart_queue, &buffer, 0); // 0 = no espera si está llena
	if (sent != pdPASS) {
		log_uart("UART → Error agregando evento a la cola uart_queue");
	}
}

