#include "task_uart.h"
#include "main.h"
#include "cmsis_os.h"
#include <string.h>
#include <stdio.h>
#include "board.h"

#define TASK_UART_STACK_SIZE 128
#define TASK_UART_PRIORITY tskIDLE_PRIORITY
#define UART_QUEUE_LENGTH 20

extern UART_HandleTypeDef huart2;

static QueueHandle_t uart_queue = NULL;

static void handle_uart_error(const char *error_msg);
static void task_uart(void *argument);

// Inicialización de la cola y la tarea UART
void init_task_uart(void) {
    // Crear la cola UART
    uart_queue = xQueueCreate(UART_QUEUE_LENGTH, sizeof(char *));
    if (uart_queue == NULL) {
        handle_uart_error("Error: No se pudo crear la cola UART\r\n");
    }
    log_uart("Cola UART creada correctamente\r\n");

    // Crear la tarea UART
    BaseType_t status = xTaskCreate(task_uart, "task_uart", TASK_UART_STACK_SIZE, NULL, TASK_UART_PRIORITY, NULL);
    if (status != pdPASS) {
        handle_uart_error("Error: No se pudo crear la tarea UART\r\n");
    }
    log_uart("Tarea UART creada correctamente\r\n");
}

// Destrucción de la cola UART
void destroy_task_uart(void) {
    if (uart_queue != NULL) {
        vQueueDelete(uart_queue);
        uart_queue = NULL;
        log_uart("Cola UART eliminada\r\n");
    }
}

// Tarea UART
void task_uart(void *argument) {
    char *msg;

    while (1) {
        if (xQueueReceive(uart_queue, &msg, portMAX_DELAY) == pdTRUE) {
            HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), HAL_MAX_DELAY);
            vPortFree(msg);  // Liberar la memoria asignada en log_uart
        }
    }
}

// Enviar un mensaje por UART
void log_uart(const char *msg) {
    if (msg == NULL || uart_queue == NULL) {
        return;
    }

    size_t len = strnlen(msg, UART_MSG_MAX_LEN - 1);
    char *copy = pvPortMalloc(len + 1);
    if (copy == NULL) {
        log_uart("Error: Memoria insuficiente para log_uart\r\n");
        return;
    }

    strncpy(copy, msg, len);
    copy[len] = '\0';

    BaseType_t sent = xQueueSend(uart_queue, &copy, pdMS_TO_TICKS(100));
    if (sent != pdPASS) {
        vPortFree(copy);  // Liberar memoria si no se pudo enviar
        log_uart("Error: No se pudo enviar el mensaje a la cola UART\r\n");
    }
}

// Manejo de errores críticos
static void handle_uart_error(const char *error_msg) {
    log_uart(error_msg);
    while (1);  // Fallback si ocurre un error crítico
}
