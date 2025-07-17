#ifndef TASK_UART_H_
#define TASK_UART_H_

#include "cmsis_os.h"

void task_uart(void *argument);
void log_uart(const char *msg);
void uart_enviar_estado_boton(void);

#endif /* TASK_UART_H_ */
