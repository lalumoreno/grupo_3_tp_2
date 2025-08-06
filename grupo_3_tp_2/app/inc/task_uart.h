#ifndef TASK_UART_H_
#define TASK_UART_H_

#define UART_MSG_MAX_LEN 64

void init_task_uart(void);
void log_uart(const char *msg);

#endif /* TASK_UART_H_ */
